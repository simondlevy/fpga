/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm>
#include <vector>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "spike.hpp"
#include <network_config.h>

namespace neuro {

    class Processor {

        private:

            // Aribtrary limts
            static constexpr int kMaxInputSpikes = 1024;
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
                output_time_ = 0;
                input_time_ = 0;
            }

            void ApplySpike(const int id, const float time, const float value)
            {
                QueuePush(LevySpike(id, time + input_time_, value));

                static LevySpike spikes_now[kQueueCapacity];
                int count = 0;

                while (!QueueIsEmpty() && QueuePeek().time == input_time_) {
                    // send these spikes as soon as they arrive to reduce latency
                    spikes_now[count] = QueuePop();
                    count++;
                }

                SendSpikes(spikes_now, count);
            }

            void ClearActivity()
            {
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

                    SendSpikes(spikes, count);

                    auto runs = run_time - input_time_;

                    while (runs > 0) {

                        const auto to_run = std::min(std::min( runs, kMaxRun),
                                kMaxRunsAhead + output_time_ - input_time_);

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

            static constexpr size_t kBytesPerMessage =
                ((kOpcodeWidth + kIndexWidth + kChargeWidth) + 7) / 8;

            typedef uint8_t Bit;

            typedef std::vector<Bit> BitArray;

            typedef uint8_t Byte;

            typedef std::vector<Byte> ByteArray;

            int input_time_;
            int output_time_;

            float output_times_[kOutputNeurons][kMaxSpikesPerNeuron];
            int output_counts_[kOutputNeurons];

            LevySpike heap_[kQueueCapacity];
            int heap_size_;

            auto GetOpcode(const uint8_t byte) -> uint8_t
            {
                return byte >> (8 - kOpcodeWidth);
            }

            auto GetRunTime(const uint8_t byte) -> uint8_t
            {
                return (((byte << kOpcodeWidth) >> kOpcodeWidth) & 0XFF);
            }

            auto GetNeuronIndex(const uint8_t byte) -> uint8_t
            {
                const uint8_t mask = 0xFF >> (8 - kOutputIndexWidth);
                return kOutputIndexWidth > 0 ? (byte >> 5) & mask : 0;
            }

            auto MakeCommand(
                    const uint8_t opcode, const uint8_t operand=0) -> uint8_t
            {
                return opcode << (8 - kOpcodeWidth) | operand;
            }

            void SendSpikes(LevySpike * spikes, int count)
            {
                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    const int charge = spike.value * kSpikeValueFactor;

                    const auto charge_twoscomp =
                        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

                    SendMessage(
                            (kOpcodeSpk << kOperandWidth) +
                            (spike.id << (kOperandWidth-kIndexWidth)) +
                            (charge_twoscomp << (kOperandWidth-kChargeWidth-1)));
                }
            }

            void SendMessage(uint64_t bits)
            {
                uint8_t bytes[kBytesPerMessage];

                for (size_t k=0; k<kBytesPerMessage; ++k) {
                    bytes[k] = (uint8_t)(bits & 0xFF);
                    bits >>= 8;
                }

                UartWrite(bytes, kBytesPerMessage);
            }

            void SendCommand(const int opcode, const int operand=0)
            {
                SendMessage((opcode << kOperandWidth) | operand);
            }

            auto ReadByte() -> uint8_t
            {
                const auto byte = UartRead();

                return byte;
            }

            void Receive()
            {
                const auto avail = UartAvailable();

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
            void UartWrite(const uint8_t * bytes, const size_t count);
            auto UartAvailable() -> int;
            auto UartRead() -> uint8_t;

    }; // class Processor

} // namespace neuro
