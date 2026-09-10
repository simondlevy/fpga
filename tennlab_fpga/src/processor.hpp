/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdio.h>

#include "output_queue.hpp"
#include "message_parser.hpp"
#include "spike.hpp"
#include "spike_heap.hpp"

namespace neuro {

    class Processor {

        private:

            static constexpr int kSystemBufferSizeBytes = 4096;
            static const size_t kMaxInputSpikes = 1024;

        public:

            Processor(
                    const int num_inputs,
                    const int num_outputs,
                    const int charge_width,
                    const int spike_value_factor,
                    const bool debug=false)
            {
                parser_ = MessageParser(num_inputs, num_outputs);

                idx_width_ = parser_.InputIndexWidth();

                charge_width_ = charge_width;
                spike_value_factor_ = spike_value_factor;
                debug_ = debug;

                const auto idx_width = parser_.InputIndexWidth();

                const auto spk_width = idx_width + charge_width;

                const auto operand_width = (
                        WidthNearestByte(parser_.OpcodeWidth() + spk_width) -
                        parser_.OpcodeWidth());

                output_time_ = 0;
                input_time_ = 0;

                const uint8_t output_size_bits = (
                        parser_.OpcodeWidth() + parser_.OutputIndexWidth());

                const int max_bytes_per_run =
                    WidthBitsToBytes(output_size_bits) * (num_outputs + 1);

                max_runs_ahead_ = kSystemBufferSizeBytes / max_bytes_per_run;

                max_run_ = std::min(
                        (1 << operand_width) - 1, max_runs_ahead_);

                opc_shift_ = 8 - parser_.OpcodeWidth();
                idx_shift_ = opc_shift_ - idx_width_;
                val_shift_ = idx_shift_ - charge_width;
            }

            void ApplySpike(const int id, const float time, const float value)
            {
                inp_queue_.Push(LevySpike(id, time + input_time_, value));

                static LevySpike spikes_now[SpikeHeap::CAPACITY];
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
                SendCommand(MessageParser::kOpcodeClr);

                Receive();

                output_time_ = 0;
                input_time_ = 0;

                inp_queue_ = SpikeHeap();
                out_queue_ = OutputQueue();
            }

            void Run(const int time)
            {
                const auto target_time = input_time_ + time;

                while (input_time_ < target_time) {

                    static LevySpike spikes[kMaxInputSpikes];
                    size_t count = 0;

                    while (true) {

                        if (inp_queue_.IsEmpty()) {
                            break;
                        }

                        const auto spike = inp_queue_.Peek();

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

                        SendCommand(MessageParser::kOpcodeRun, to_run);

                        input_time_ += runs;

                        runs -= to_run;
                    }

                    if (run_time == target_time) {

                        SendCommand(MessageParser::kOpcodeSnc);
                    }
                }

                Receive();
            }

            auto GetOutputCount(const int out_idx) -> int
            {
                return out_queue_.counts[out_idx];
            }

            // Hardware-dependent implementation
            void Connect();

        private:

            const size_t MAXMSG = 32;

            int idx_width_;
            int charge_width_;
            int spike_value_factor_;
            bool debug_;
            int input_time_;
            int output_time_;
            int max_runs_ahead_;
            int max_run_;
            uint8_t opc_shift_;
            uint8_t idx_shift_;
            uint8_t val_shift_;

            MessageParser parser_;

            SpikeHeap inp_queue_;

            OutputQueue out_queue_;

            void PrepareToSend(LevySpike * spikes, int count)
            {
                for (int k=0; k<count; ++k) {

                    const auto spike = spikes[k];

                    const uint8_t idx_mask = (1 << idx_width_) - 1;
                    const uint8_t val_mask = (1 << charge_width_) - 1;

                    const int8_t val = (int8_t)(spike.value * spike_value_factor_);

                    const uint8_t byte =
                        MessageParser::kOpcodeSpk << opc_shift_ |
                        (spike.id & idx_mask) << idx_shift_ |
                        (val & val_mask) << val_shift_;

                    WriteByte(byte);
                }
            }

            void SendCommand(const uint8_t opcode, const uint8_t operand=0)
            {
                WriteByte(parser_.MakeCommand(opcode, operand));
            }

            void WriteByte(const uint8_t byte)
            {
                if (debug_) {
                    printf("DEBUG: write x%02X\n", byte);
                }

                Write(byte);
            }

            auto ReadByte() -> uint8_t
            {
                const auto byte = Read();

                if (debug_) {
                    printf("DEBUG: read  x%02X\n", byte);
                }

                return byte;
            }

            void Receive()
            {
                const auto avail = Available();

                if (debug_) {
                    printf("DEBUG: avail %u\n", avail);
                }

                for (size_t k=0; k<avail; ++k) {
                    
                    const auto byte = ReadByte();

                    const auto opcode = parser_.GetOpcode(byte);

                    if (opcode == MessageParser::kOpcodeRun) {
                        const uint8_t operand = parser_.GetRunTime(byte);
                        output_time_ += operand;
                    }

                    else if (opcode == MessageParser::kOpcodeSpk) {
                        const auto out_idx = parser_.GetNeuronIndex(byte);
                        out_queue_.Append(out_idx, (float)output_time_);
                    }
                }
            }

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

            // Hardware-dependent implementation
            void Write(const uint8_t byte);
            auto Available() -> size_t;
            auto Read() -> uint8_t;


    }; // class Processor

} // namespace neuro
