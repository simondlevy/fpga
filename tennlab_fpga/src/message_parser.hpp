/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <math.h>

namespace neuro {

    class MessageParser {

        public:

            enum {
                kOpcodeRun,
                kOpcodeSpk,
                kOpcodeSnc,
                kOpcodeClr,
                kOpcodeCount
            };

            MessageParser() = default;

            MessageParser(const int num_inputs, const int num_outputs)
            {
                num_inputs_ = num_inputs;
                num_outputs_ = num_outputs;

                opcode_width_ = UnsignedWidth(kOpcodeCount - 1);

                output_idx_width_ = UnsignedWidth(num_outputs - 1) ;
            }

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
                return UnsignedWidth(num_inputs_ - 1) ;
            }

            auto OutputIndexWidth() -> uint8_t
            {
                return UnsignedWidth(num_outputs_ - 1) ;
            }

            auto MakeCommand(
                    const uint8_t opcode, const uint8_t operand=0) -> uint8_t
            {
                return opcode << (8 - opcode_width_) | operand;
            }

        private:

            int num_inputs_;
            int num_outputs_;
            int output_idx_width_;
            int opcode_width_;

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

    }; // class MessageParser

} // namespace neuro
