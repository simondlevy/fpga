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

#include "serial.h"
#include "threading.h"

#include "output_queue.hpp"
#include "spike.hpp"
#include "spike_heap.hpp"

namespace neuro {

    class FpgaConnection {

        private:

            static constexpr int kSystemBufferSizeBytes = 4096;
            static const size_t kMaxInputSpikes = 1024;

            enum {
                kOpcodeRun,
                kOpcodeSpk,
                kOpcodeSnc,
                kOpcodeClr,
                kOpcodeCount
            };

        public:

            FpgaConnection(
                    const int num_inputs,
                    const int num_outputs,
                    const int charge_width,
                    const float clock_freq,
                    const int spike_value_factor)
            {
                Serial::Begin();

                num_inputs_ = num_inputs;
                num_outputs_ = num_outputs;

                spike_value_factor_ = spike_value_factor;

                opcode_width_ = UnsignedWidth(kOpcodeCount - 1);

                charge_width_ = charge_width;

                const auto idx_width = UnsignedWidth(num_inputs - 1);

                const auto spk_width = idx_width + charge_width;

                const auto operand_width = (
                        WidthNearestByte(opcode_width_ + spk_width) -
                        opcode_width_);

                output_time_ = 0;
                input_time_ = 0;

                output_idx_width_ = UnsignedWidth(num_outputs - 1) ;

                const uint8_t output_size_bits =(
                        opcode_width_ + output_idx_width_);

                const int max_bytes_per_run =
                    WidthBitsToBytes(output_size_bits) * (num_outputs + 1);

                secs_per_run_ = ((float)num_outputs / clock_freq) +
                    max_bytes_per_run * 10.f / Serial::GetBaudRate();

                max_runs_ahead_ = kSystemBufferSizeBytes / max_bytes_per_run;

                max_run_ = std::min(
                        (1 << operand_width) - 1, max_runs_ahead_);
            }

            void ApplySpike(const Spike & spike)
            {
                inp_queue_.Push(Spike(spike.id, spike.time +
                            input_time_, spike.value));

                static Spike spikes_now[SpikeHeap::CAPACITY];
                int count = 0;

                while (!inp_queue_.IsEmpty() &&
                        inp_queue_.Peek().time == input_time_) {
                    // send these spikes as soon as they arrive to reduce latency
                    spikes_now[count] = inp_queue_.Pop();
                    count++;
                }

                PrepareToSend(spikes_now, count);
            }

            void ClearActivity()
            {
                SendCommand(kOpcodeClr);

                /*
                if (Serial::Available() != 1) {
                    printf("Error in ClearActivity()\n");
                    return;
                }

                const auto byte = Serial::Read();
                const auto opcode = GetOpcode(byte);
                if (opcode != kOpcodeClr) {
                    printf("Failed to clear activity\n");
                }*/

                output_time_ = 0;
                input_time_ = 0;

                inp_queue_ = SpikeHeap();
                out_queue_ = OutputQueue();
            }

            void Run(const int time)
            {
                const auto target_time = input_time_ + time;

                while (input_time_ < target_time) {

                    static Spike spikes[kMaxInputSpikes];
                    size_t count = 0;

                    while (true) {

                        if (inp_queue_.IsEmpty()) {
                            break;
                        }

                        const auto spike = inp_queue_.Peek();

                        // Dump(spike);

                        if (spike.time != input_time_) {
                            break;
                        }

                        spikes[count] = inp_queue_.Pop();
                        count++;

                    }

                    const auto run_time = !inp_queue_.IsEmpty() ?
                        (int)inp_queue_.Peek().time :
                        target_time;

                    PrepareToSend(spikes, count);

                    auto runs = run_time - input_time_;

                    while (runs > 0) {

                        const auto to_run = std::min(std::min(
                                    runs,
                                    max_run_),
                                    max_runs_ahead_ + output_time_ - input_time_);

                        if (to_run == 0) {
                            //Thread::yield();
                            //continue;
                        }

                        SendCommand(kOpcodeRun, to_run);

                        input_time_ += runs;

                        //Thread::sleep(secs_per_run_ * runs);

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
                return out_queue_.counts[out_idx];
            }


        private:

            const size_t MAXMSG = 32;

            int input_time_;
            int output_time_;
            int output_idx_width_;
            int num_inputs_;
            int num_outputs_;
            int opcode_width_;
            int charge_width_;
            int spike_value_factor_;
            float secs_per_run_;
            int max_runs_ahead_;
            int max_run_;

            SpikeHeap inp_queue_;

            OutputQueue out_queue_;

            void PrepareToSend(Spike * spikes, int count)
            {
                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    //Dump(spike);

                    const uint8_t byte = (kOpcodeSpk << (opcode_width_ + charge_width_ - 1) |
                            (spike.id << charge_width_) |
                            int(spike.value*spike_value_factor_));

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                WriteByte(opcode << (8 - opcode_width_) | operand);
            }

            void WriteByte(const uint8_t byte)
            {
                Serial::Write(byte);
            }

            void Receive()
            {
                while (Serial::Available() > 0) {
                    printf("x%02X\n", Serial::Read());
                }

                /*
                while (true) {

                    const auto byte = Serial::Read();

                    const auto opcode = GetOpcode(byte);

                    printf("received opcode=x%02X\n", opcode);

                    if (opcode == kOpcodeRun) {
                        const uint8_t operand =
                            (((byte << opcode_width_) >> opcode_width_) & 0XFF);
                    }

                    else if (opcode == kOpcodeSpk) {
                        const auto idx_width = output_idx_width_;
                        const uint8_t mask = 0xFF >> (8 - idx_width);
                        const auto out_idx = idx_width > 0 ? (byte >> 5) & mask : 0;
                        out_queue_.append(out_idx, (float)output_time_);
                    }

                    else if (opcode == kOpcodeSnc) {
                        break;
                    }

                    else {
                        printf("received bad opcode: x%02X\n", opcode);
                    }
                }*/
            }

            auto GetOpcode(const uint8_t byte) -> uint8_t
            {
                return byte >> (8 - opcode_width_);
            }

            static void Dump(const Spike & spike)
            {
                printf("id=%d time=%f value=%f\n", spike.id, spike.time, spike.value);

            }

            // Bit-twiddling -------------------------------------------------

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

    }; // class FpgaConnection

} // namespace neuro
