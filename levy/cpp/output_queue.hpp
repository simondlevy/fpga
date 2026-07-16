#pragma once

namespace neuro {

    class OutputQueue {

        private:

            static constexpr size_t MAX_NEURONS = 16;
            static constexpr size_t MAX_SPIKES_PER_NEURON = 256;

        public:

            float times[MAX_NEURONS][MAX_SPIKES_PER_NEURON];
            size_t counts[MAX_NEURONS];

            OutputQueue() = default;

            OutputQueue& operator=(const OutputQueue& other) = default;

            void append(const int index, const float time)
            {
                times[index][counts[index]] = time;
                counts[index]++;
            }

    }; // class OutputQueue


} // namespace neuro
