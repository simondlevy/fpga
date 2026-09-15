/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm>

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
                if (GetDebug()) {
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
                if (GetDebug()) {
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
                if (GetDebug()) {
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

                        const auto to_run = std::min(std::min( runs, GetMaxRun()),
                                GetMaxRunsAhead() + output_time_ - input_time_);

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

            int input_time_;
            int output_time_;

            float output_times_[kOutputNeurons][kMaxSpikesPerNeuron];
            int output_counts_[kOutputNeurons];

            LevySpike heap_[kQueueCapacity];
            int heap_size_;

            auto GetOpcode(const uint8_t byte) -> uint8_t
            {
                return byte >> (8 - GetOpcodeWidth());
            }

            auto GetRunTime(const uint8_t byte) -> uint8_t
            {
                return (((byte << GetOpcodeWidth()) >> GetOpcodeWidth()) & 0XFF);
            }

            auto GetNeuronIndex(const uint8_t byte) -> uint8_t
            {
                const uint8_t mask = 0xFF >> (8 - GetOutputIndexWidth());
                return GetOutputIndexWidth() > 0 ? (byte >> 5) & mask : 0;
            }

            auto MakeCommand(
                    const uint8_t opcode, const uint8_t operand=0) -> uint8_t
            {
                return opcode << (8 - GetOpcodeWidth()) | operand;
            }

             void PrepareToSend(LevySpike * spikes, int count)
            {
                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    const uint8_t idx_mask = (1 << GetInputIndexWidth()) - 1;
                    const uint8_t val_mask = (1 << GetChargeWidth()) - 1;

                    const int8_t val = (int8_t)(spike.value * GetSpikeValueFactor());

                    const uint8_t byte =
                        kOpcodeSpk << GetOpcodeShift() |
                        (spike.id & idx_mask) << GetIndexShift() |
                        (val & val_mask) << GetValueShift();

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                WriteByte(MakeCommand(opcode, operand));
            }

            void WriteByte(const uint8_t byte)
            {
                if (GetDebug()) {
                    printf("  write x%02X\n", byte);
                }

                UartWrite(byte);
            }

            auto ReadByte() -> uint8_t
            {
                const auto byte = UartRead();

                if (GetDebug()) {
                    printf("  read  x%02X\n", byte);
                }

                return byte;
            }

            void Receive()
            {
                const auto avail = UartAvailable();

                if (GetDebug()) {
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

            // Network-dependent, generated by compiling----------------------

            int GetChargeWidth();
            int GetSpikeValueFactor();
            int GetOpcodeWidth();
            int GetInputIndexWidth();
            int GetOutputIndexWidth();
            int GetOpcodeShift();
            int GetIndexShift();
            int GetValueShift();
            int GetMaxRunsAhead();
            int GetMaxRun();
            bool GetDebug();

            // Hardware-dependent --------------------------------------------

            void UartBegin();
            void UartWrite(const uint8_t byte);
            auto UartAvailable() -> int;
            auto UartRead() -> uint8_t;

    }; // class Processor

} // namespace neuro
