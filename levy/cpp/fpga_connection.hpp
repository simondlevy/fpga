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

            static const size_t MAX_SPIKE_TIME = 100;
            static const size_t MAX_INPUT_NEURONS = 10;

            enum {
                OPCODE_RUN,
                OPCODE_SPK,
                OPCODE_SNC,
                OPCODE_CLR,
                OPCODE_COUNT
            };

            static constexpr int kSystemBufferSizeBytes = 4096;

        public:

            FpgaConnection(
                    Serial & serial,
                    const int num_inputs,
                    const int num_outputs,
                    const int charge_width,
                    const float clock_freq,
                    const int spike_value_factor)
            {
                serial_ = serial;

                num_inputs_ = num_inputs;
                num_outputs_ = num_outputs;

                spike_value_factor_ = spike_value_factor;

                opcode_width_ = UnsignedWidth(OPCODE_COUNT - 1);

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
                    max_bytes_per_run * 10.f / serial.GetBaudRate();

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
                SendCommand(OPCODE_CLR);

                serial_.Flush();

                Receive();

                output_time_ = 0;
                input_time_ = 0;

                inp_queue_ = SpikeHeap();
                out_queue_ = OutputQueue();
            }

            void Run(const int time)
            {
                Thread::start(ThreadCallback, this);

                const auto target_time = input_time_ + time;
            }

            auto GetOutputCount(const int out_idx) -> int
            {
                for (int k=0; k<out_queue_.counts[out_idx]; ++k) {
                    printf("%f ", out_queue_.times[out_idx][k]);
                }
                printf("\n");

                return out_queue_.counts[out_idx];
            }


        private:

            const size_t MAXMSG = 32;

            Serial serial_;

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

                    const uint8_t byte = (OPCODE_SPK << (opcode_width_ + charge_width_ - 1) |
                            (spike.id << charge_width_) |
                            int(spike.value*spike_value_factor_));

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                //printf("SendCommand: %d %d\n", opcode, operand);
                WriteByte(opcode << (8 - opcode_width_) | operand);
            }

            void WriteByte(const uint8_t byte)
            {
                //printf("WriteByte: x%02X\n", byte);
                serial_.Write(&byte, 1);
            }

            void Receive()
            {
                printf("receive\n");

                while (true) {

                    uint8_t byte = 0;

                    const auto rxlen = serial_.Read(&byte, 1);

                    const auto idx_width = output_idx_width_;

                    const uint8_t opcode = byte >> (8 - opcode_width_);

                    const uint8_t operand = (((byte << opcode_width_) >> opcode_width_) &
                            0XFF);

                    printf("x%02X x%02X\n" , opcode, operand);

                    switch (opcode) {

                        case OPCODE_RUN:
                            //ran = operand;
                            //self._output_time += ran;
                            break;

                        case OPCODE_SPK:
                            //mask = 0xFF >> (8 - idx_width);
                            //out_idx = ((byte >> 5) & mask) if idx_width > 0 else 0;
                            //time = float(self._output_time);
                            //self._out_queue.append(out_idx, time);
                            break;

                        case OPCODE_SNC:
                            break;

                        case OPCODE_CLR:
                            break;
                    }

                    break;
                }
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

             static void * ThreadCallback(void * arg)
            {
                ((FpgaConnection *)arg)->Receive();

                return NULL;
            }

            static void Dump(const Spike & spike)
            {
                printf("id=%d time=%f value=%f\n", spike.id, spike.time, spike.value);

            }

    }; // class FpgaConnection

} // namespace neuro
