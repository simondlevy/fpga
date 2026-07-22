/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "spike.hpp"

namespace neuro {

    class SpikeHeap {

        friend class FpgaConnection;

        private:

            static constexpr int CAPACITY = 1024;

            FpgaSpike heap_[CAPACITY];
            int size_;

            SpikeHeap()
            {
                size_ = 0;
            }

            void Push(const FpgaSpike & spike)
            {
                // Insert at the end and sift up
                heap_[size_] = spike;
                size_++;
                SiftUp(size_ - 1);
            }

            auto Pop() -> FpgaSpike
            {
                const auto min_val = heap_[0];

                // Move the last element to the root and sift down
                heap_[0] = heap_[size_ - 1];
                size_ -= 1;

                if (size_ > 0) {
                    SiftDown(0);
                }

                return min_val;
            }

            auto Peek() -> FpgaSpike
            {
                return heap_[0];
            }

            auto IsEmpty() -> bool
            {
                return size_ == 0;
            }

            auto Size() -> int
            {
                return size_;
            }

        private:

            void SiftUp(int idx)
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

            void SiftDown(int idx)
            {
                // Maintains min-heap property by moving an element downwards.
                while (true) {

                    const auto left = 2 * idx + 1;
                    const auto right = 2 * idx + 2;
                    auto smallest = idx;

                    if (left < size_ && heap_[left] < heap_[smallest]) {
                        smallest = left;
                    }

                    if (right < size_ && heap_[right] < heap_[smallest]) {
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

    }; // class Heap

} // namespace neuro
