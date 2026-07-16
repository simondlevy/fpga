/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace neuro {

    class IoConfig {

        friend class FpgaConnection;

        private:

            uint8_t usable_charge_width;
            uint8_t idx_width;
            uint8_t num_bytes;
            uint8_t opcode_width;
            uint8_t operand_width;
            float time;

            IoConfig() = default;

            IoConfig(
                    const int opcode_count,
                    const int num_neurons,
                    const int charge_width,
                    const int usable_charge_width,
                    const bool is_axi=true)
                : usable_charge_width(usable_charge_width)
            {
                opcode_width = UnsignedWidth(opcode_count - 1);

                idx_width = UnsignedWidth(num_neurons - 1);

                const auto spk_width = idx_width + usable_charge_width;

                operand_width = is_axi ?
                    WidthNearestByte(opcode_width + spk_width) - opcode_width :
                    spk_width;

                Clear();

                num_bytes = WidthBitsToBytes(
                        opcode_width + idx_width + usable_charge_width);
            }

            void Clear()
            {
                time = 0;
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

    }; // class IOConfig

} // namespace neuro
