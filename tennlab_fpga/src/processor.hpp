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
#include "queue.hpp"

// auto-generated
#include "network_config.h"

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
                queue_.Push(LevySpike(id, time + input_time_, value));

                static LevySpike spikes_now[SpikeQueue::kCapacity];
                int count = 0;

                while (!queue_.IsEmpty() && queue_.Peek().time == input_time_) {
                    // send these spikes as soon as they arrive to reduce latency
                    spikes_now[count] = queue_.Pop();
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

                        if (queue_.IsEmpty()) {
                            break;
                        }

                        const auto spike = queue_.Peek();

                        if (spike.time != input_time_) {
                            break;
                        }

                        spikes[count] = queue_.Pop();
                        count++;

                    }

                    const auto run_time = !queue_.IsEmpty() ?
                        (int)queue_.Peek().time :
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

            static constexpr size_t kBytesPerMessageToFpga =
                ((kOpcodeWidth + kIndexWidth + kChargeWidth) + 7) / 8;

            typedef uint8_t Bit;

            typedef std::vector<Bit> BitArray;

            typedef uint8_t Byte;

            typedef std::vector<Byte> ByteArray;

            int input_time_;
            int output_time_;

            float output_times_[kOutputNeurons][kMaxSpikesPerNeuron];
            int output_counts_[kOutputNeurons];

            SpikeQueue queue_;

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
                uint8_t bytes[kBytesPerMessageToFpga];

                for (size_t k=0; k<kBytesPerMessageToFpga; ++k) {
                    bytes[k] = (uint8_t)(bits & 0xFF);
                    bits >>= 8;
                }

                UartWrite(bytes, kBytesPerMessageToFpga);
            }

            void SendCommand(const int opcode, const int operand=0)
            {
                SendMessage((opcode << kOperandWidth) | operand);
            }

            void Receive()
            {
                const auto avail = UartAvailable();

                for (int k=0; k<avail; ++k) {

                    const auto byte = UartRead();

                    const auto opcode = byte >> (8 - kOpcodeWidth);

                    if (opcode == kOpcodeRun) {
                        const uint8_t operand = 
                            (((byte << kOpcodeWidth) >> kOpcodeWidth) & 0XFF);
                        output_time_ += operand;
                    }

                    else if (opcode == kOpcodeSpk) {

                        const uint8_t mask = 0xFF >> (8 - kOutputNeuronIndexWidth);

                        const auto out_idx =
                            kOutputNeuronIndexWidth > 0 ? (byte >> 5) & mask : 0;

                        output_times_[out_idx][output_counts_[out_idx]] = output_time_;
                        output_counts_[out_idx]++;
                    }
                }
            }

            // Hardware-dependent --------------------------------------------

            void UartBegin();
            void UartWrite(const uint8_t * bytes, const size_t count);
            auto UartAvailable() -> int;
            auto UartRead() -> uint8_t;

    }; // class Processor

} // namespace neuro
