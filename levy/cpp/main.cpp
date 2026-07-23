/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "fpga_connection.hpp"

static constexpr int kNumInputs = 2;
static constexpr int kNumOutputs = 2;
static constexpr float kClockFreq = 100000000;
static constexpr int kChargeWidth = 5;
static constexpr int kEntryValueFactor = 10;

class Entry {

    public:

        int step;
        int id;
        float time;
        float value;

        Entry(
                const int step,
                const int id,
                const float time,
                const float value)
            : step(step), id(id), time(time), value(value) {}

        Entry() = default;

        Entry(const Entry & other) = default;
};

static auto parse(std::stringstream & ss) -> std::string
{
    std::string token;
    std::getline(ss, token, ' ');
    return token;
}

static auto parseint(std::stringstream & ss) -> int
{
    return std::stoi(parse(ss));
}

static auto parsefloat(std::stringstream & ss) -> float
{
    return std::stof(parse(ss));
}

int main()
{
    std::ifstream file("../spikes.txt"); 

    std::vector<Entry> data;             

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    auto fpga = neuro::FpgaConnection(
            kNumInputs,
            kNumOutputs,
            kChargeWidth,
            kClockFreq,
            kEntryValueFactor);

    (void)fpga;

    while (true) {

        std::string line;

        if (!std::getline(file, line)) {
            break;
        }

        std::stringstream ss(line);

        std::string token;

        const auto step = parseint(ss);
        const auto id = parseint(ss);
        const auto time = parsefloat(ss);
        const auto value = parsefloat(ss);

        data.push_back(Entry(step, id, time, value));

        std::cout << step << " " << id << " " << time << " " << value << std::endl;

    }

    file.close();

    return 0;

    /*
    for (uint8_t k=0; k<2; ++k) {

        fpga.ClearActivity();

        for (int i=0; i<28; ++i) {
            fpga.ApplyEntry(0, i, 1.0);
        }

        for (int i=0; i<26; ++i) {
            fpga.ApplyEntry(1, i, 1.0);
        }

        fpga.Run(50);

        printf("%d %d\n\n", fpga.GetOutputCount(0), fpga.GetOutputCount(1));
    }*/

    file.close(); 

    return 0;
}
