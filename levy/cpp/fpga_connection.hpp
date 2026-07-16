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

#include "ioconfig.hpp"
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

                opcode_width_ = IoConfig::UnsignedWidth(OPCODE_COUNT - 1);

                inp_config_ = IoConfig(OPCODE_COUNT, num_inputs, charge_width, charge_width);

                out_config_ = IoConfig(OPCODE_COUNT, num_outputs, charge_width, 0);

                const uint8_t output_size_bits =(
                        out_config_.opcode_width + out_config_.idx_width);

                const int max_bytes_per_run =
                    IoConfig::WidthBitsToBytes(output_size_bits) * (num_outputs + 1);

                secs_per_run_ = ((float)num_outputs / clock_freq) +
                    max_bytes_per_run * 10.f / serial.GetBaudRate();

                max_runs_ahead_ = kSystemBufferSizeBytes / max_bytes_per_run;

                max_run_ = std::min(
                        (1 << inp_config_.operand_width) - 1,
                        max_runs_ahead_);
            }

            void ApplySpike(const Spike & spike)
            {
                inp_queue_.Push(Spike(spike.id, spike.time +
                            inp_config_.time, spike.value));

                static Spike spikes_now[SpikeHeap::CAPACITY];
                int count = 0;

                while (!inp_queue_.IsEmpty() &&
                        inp_queue_.Peek().time == inp_config_.time) {
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

                inp_config_.Clear();
                out_config_.Clear();

                inp_queue_ = SpikeHeap();
                out_queue_ = OutputQueue();
            }

            void Run(const int time)
            {
                const auto target_time = inp_config_.time + time;

                Thread::start();
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

            int num_inputs_;
            int num_outputs_;
            int opcode_width_;
            int charge_width_;
            int spike_value_factor_;
            float secs_per_run_;
            int max_runs_ahead_;
            int max_run_;

            IoConfig inp_config_;
            IoConfig out_config_;

            SpikeHeap inp_queue_;

            OutputQueue out_queue_;

            void PrepareToSend(Spike * spikes, int count)
            {
                const auto chgwidth = inp_config_.usable_charge_width;

                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    const uint8_t byte = (OPCODE_SPK << (opcode_width_ + chgwidth - 1) |
                            (spike.id << chgwidth) |
                            int(spike.value*spike_value_factor_));

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                //printf("SendCommand: %d %d\n", opcode, operand);
                WriteByte(opcode << (8 - inp_config_.opcode_width) | operand);
            }

            void WriteByte(const uint8_t byte)
            {
                //printf("WriteByte: x%02X\n", byte);
                serial_.Write(&byte, 1);
            }

            void Receive()
            {
                const auto num_rx_bytes = out_config_.num_bytes;

                while (true) {

                    uint8_t msg[MAXMSG] = {};

                    const auto rxlen = serial_.Read(msg, num_rx_bytes);

                    if (rxlen != num_rx_bytes) {
                        printf("Did not receive coherent response from target.\n");
                        break;
                    }

                    break;
                }
            }

            void Dump(const Spike & spike)
            {
                printf("id=%d time=%f value=%f\n", spike.id, spike.time, spike.value);

            }

    }; // class FpgaConnection

} // namespace neuro
