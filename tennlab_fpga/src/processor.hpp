/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "spike.hpp"
#include <network_config.h>

namespace neuro {

    class Processor {

        private:

            static constexpr int kSystemBufferSizeBytes = 4096;
            static constexpr int kMaxInputSpikes = 1024;
            static constexpr int kMaxOutputNeurons = 16;
            static constexpr int kMaxSpikesPerNeuron = 256;
            static constexpr int kQueueCapacity = 1024;

            enum {
                kOpcodeRun,
                kOpcodeSpk,
                kOpcodeSnc,
                kOpcodeClr,
                kOpcodeCount
            };

        public:

            Processor()
            {
                opcode_width_ = UnsignedWidth(kOpcodeCount - 1);
                output_idx_width_ = UnsignedWidth(kNumOutputs - 1) ;
 
                idx_width_ = InputIndexWidth();

                const auto idx_width = InputIndexWidth();

                const auto spk_width = idx_width + kChargeWidth;

                operand_width_ = WidthNearestByte(OpcodeWidth() + spk_width)
                        - OpcodeWidth();

                output_time_ = 0;
                input_time_ = 0;

                const uint8_t output_size_bits =
                    OpcodeWidth() + OutputIndexWidth();

                const int max_bytes_per_run =
                    WidthBitsToBytes(output_size_bits) * (kNumOutputs + 1);

                max_runs_ahead_ = kSystemBufferSizeBytes / max_bytes_per_run;

                max_run_ = std::min(
                        (1 << operand_width_) - 1, max_runs_ahead_);

                opc_shift_ = 8 - OpcodeWidth();
                idx_shift_ = opc_shift_ - idx_width_;
                val_shift_ = idx_shift_ - kChargeWidth;
            }

            void ApplySpike(const int id, const float time, const float value)
            {
                if (kDebug) {
                    printf("AS\n");
                }

                QueuePush(LevySpike(id, time + input_time_, value));

                static LevySpike spikes_now[kQueueCapacity];
                int count = 0;

                while (!QueueIsEmpty() && QueuePeek().time == input_time_) {
                    // send these spikes as soon as they arrive to reduce latency
                    spikes_now[count] = QueuePop();
                    count++;
                }

                PrepareToSend(spikes_now, count);
            }

            void ClearActivity()
            {
                if (kDebug) {
                    printf("CLR\n");
                }

                SendCommand(kOpcodeClr);

                Receive();

                output_time_ = 0;
                input_time_ = 0;

                heap_size_ = 0;

                memset(output_times_, 0, sizeof(output_times_));
                memset(output_counts_, 0, sizeof(output_counts_));
            }

            void Run(const int time)
            {
                if (kDebug) {
                    printf("RUN\n");
                }

                const auto target_time = input_time_ + time;

                while (input_time_ < target_time) {

                    static LevySpike spikes[kMaxInputSpikes];
                    int count = 0;

                    while (true) {

                        if (QueueIsEmpty()) {
                            break;
                        }

                        const auto spike = QueuePeek();

                        if (spike.time != input_time_) {
                            break;
                        }

                        spikes[count] = QueuePop();
                        count++;

                    }

                    const auto run_time = !QueueIsEmpty() ?
                        (int)QueuePeek().time :
                        target_time;

                    PrepareToSend(spikes, count);

                    auto runs = run_time - input_time_;

                    while (runs > 0) {

                        const auto to_run = std::min(std::min(
                                    runs,
                                    max_run_),
                                max_runs_ahead_ + output_time_ - input_time_);

                        SendCommand(kOpcodeRun, to_run);

                        input_time_ += runs;

                        runs -= to_run;
                    }

                    if (run_time == target_time) {

                        SendCommand(kOpcodeSnc);
                    }
                }

                Receive();
            }

            auto GetOutputCount(const int out_idx) -> int
            {
                return output_counts_[out_idx];
            }

            void Connect()
            {
                UartBegin();
                ClearActivity();
            }

        private:

            const int MAXMSG = 32;

            int idx_width_;
            bool kDebug;
            int input_time_;
            int output_time_;
            int max_runs_ahead_;
            int max_run_;
            uint8_t operand_width_;
            uint8_t opc_shift_;
            uint8_t idx_shift_;
            uint8_t val_shift_;
            int output_idx_width_;
            int opcode_width_;

            float output_times_[kMaxOutputNeurons][kMaxSpikesPerNeuron];
            int output_counts_[kMaxOutputNeurons];

            LevySpike heap_[kQueueCapacity];
            int heap_size_;

            auto GetOpcode(const uint8_t byte) -> uint8_t
            {
                return byte >> (8 - opcode_width_);
            }

            auto GetRunTime(const uint8_t byte) -> uint8_t
            {
                return (((byte << opcode_width_) >> opcode_width_) & 0XFF);
            }

            auto GetNeuronIndex(const uint8_t byte) -> uint8_t
            {
                const auto idx_width = output_idx_width_;
                const uint8_t mask = 0xFF >> (8 - idx_width);
                return idx_width > 0 ? (byte >> 5) & mask : 0;
            }

            auto OpcodeWidth() -> uint8_t
            {
                return opcode_width_;
            }

            auto InputIndexWidth() -> uint8_t
            {
                return UnsignedWidth(kNumInputs - 1) ;
            }

            auto OutputIndexWidth() -> uint8_t
            {
                return UnsignedWidth(kNumOutputs - 1) ;
            }

            auto MakeCommand(
                    const uint8_t opcode, const uint8_t operand=0) -> uint8_t
            {
                return opcode << (8 - opcode_width_) | operand;
            }

             void PrepareToSend(LevySpike * spikes, int count)
            {
                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    const uint8_t idx_mask = (1 << idx_width_) - 1;
                    const uint8_t val_mask = (1 << kChargeWidth) - 1;

                    const int8_t val = (int8_t)(spike.value * kSpikeValueFactor);

                    const uint8_t byte =
                        kOpcodeSpk << opc_shift_ |
                        (spike.id & idx_mask) << idx_shift_ |
                        (val & val_mask) << val_shift_;

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                WriteByte(MakeCommand(opcode, operand));
            }

            void WriteByte(const uint8_t byte)
            {
                if (kDebug) {
                    printf("  write x%02X\n", byte);
                }

                UartWrite(byte);
            }

            auto ReadByte() -> uint8_t
            {
                const auto byte = UartRead();

                if (kDebug) {
                    printf("  read  x%02X\n", byte);
                }

                return byte;
            }

            void Receive()
            {
                const auto avail = UartAvailable();

                if (kDebug) {
                    printf("  avail %d\n", (int)avail);
                }

                for (int k=0; k<avail; ++k) {
                    
                    const auto byte = ReadByte();

                    const auto opcode = GetOpcode(byte);

                    if (opcode == kOpcodeRun) {
                        const uint8_t operand = GetRunTime(byte);
                        output_time_ += operand;
                    }

                    else if (opcode == kOpcodeSpk) {
                        const auto out_idx = GetNeuronIndex(byte);
                        output_times_[out_idx][output_counts_[out_idx]] = output_time_;
                        output_counts_[out_idx]++;
                    }
                }
            }

            void QueuePush(const LevySpike & spike)
            {
                // Insert at the end and sift up
                heap_[heap_size_] = spike;
                heap_size_++;
                QueueSiftUp(heap_size_ - 1);
            }

            auto QueuePop() -> LevySpike
            {
                const auto min_val = heap_[0];

                // Move the last element to the root and sift down
                heap_[0] = heap_[heap_size_ - 1];
                heap_size_ -= 1;

                if (heap_size_ > 0) {
                    QueueSiftDown(0);
                }

                return min_val;
            }

            void QueueSiftUp(int idx)
            {
                // Maintains min-heap property by moving an element upwards.
                while (idx > 0) {

                    const auto parent = (idx - 1) / 2;

                    if (!(heap_[idx]< heap_[parent])) {
                        break;
                    }

                    const auto tmp = heap_[idx];
                    heap_[idx] = heap_[parent];
                    heap_[parent] = tmp;

                    idx = parent;
                }
            }

            void QueueSiftDown(int idx)
            {
                // Maintains min-heap property by moving an element downwards.
                while (true) {

                    const auto left = 2 * idx + 1;
                    const auto right = 2 * idx + 2;
                    auto smallest = idx;

                    if (left < heap_size_ && heap_[left] < heap_[smallest]) {
                        smallest = left;
                    }

                    if (right < heap_size_ && heap_[right] < heap_[smallest]) {
                        smallest = right;
                    }

                    if (smallest == idx) {
                        break;
                    }

                    const auto tmp = heap_[idx];
                    heap_[idx] = heap_[smallest];
                    heap_[smallest] = tmp;

                    idx = smallest;
                }
            }

            auto QueuePeek() -> LevySpike
            {
                return heap_[0];
            }

            auto QueueIsEmpty() -> bool
            {
                return heap_size_ == 0;
            }

 
            // Hardware-dependent --------------------------------------------

            void UartBegin();
            void UartWrite(const uint8_t byte);
            auto UartAvailable() -> int;
            auto UartRead() -> uint8_t;

            // Bit-twiddling -------------------------------------------------

            static auto WidthNearestByte(const int bits) -> int
            {
                return WidthBytesToBits(WidthBitsToBytes(bits));
            }

            static auto WidthBitsToBytes(const int bits) -> int
            {
                return int(ceil(bits / 8.f));
            }

            static auto WidthBytesToBits(const int bytes) -> int
            {
                return bytes * 8;
            }

            static auto UnsignedWidth(const int value) -> int
            {
                return SignedWidth(value) - 1;
            }

            static auto SignedWidth(const int value) -> int
            {
                return Clog2(abs(value) + int(value >= 0)) + 1;
            }

            static auto Clog2(float value) -> int
            { 
                return int(ceil(log2(value)));
            }


    }; // class Processor

} // namespace neuro
