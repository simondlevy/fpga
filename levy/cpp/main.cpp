/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

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

static auto loaddata(const std::string filename) -> std::vector<Entry>
{
    std::ifstream file(filename);

    std::vector<Entry> data;             

    if (!file.is_open()) {
        std::cerr << "Error opening file " << filename << std::endl;
        exit(1);
    }

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
    }

    file.close();

    return data;
}

int main()
{
    auto fpga = neuro::FpgaConnection(
            kNumInputs,
            kNumOutputs,
            kChargeWidth,
            kClockFreq,
            kEntryValueFactor,
            true);

    const auto data = loaddata("../spikes.txt"); 

    // const auto runtime = data.back().step + 1;

    for (int timestep=0; timestep<66/*runtime*/; ++timestep) {

        fpga.ClearActivity();

        for (auto entry:data) {
            if (entry.step == timestep) {
                fpga.ApplySpike(entry.id, entry.time, entry.value);
            }
        }

        fpga.Run(50);

        printf("%03d: %02d %02d\n\n",
                timestep, fpga.GetOutputCount(0), fpga.GetOutputCount(1));
    }

    return 0;
}
