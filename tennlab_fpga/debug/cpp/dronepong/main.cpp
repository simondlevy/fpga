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

#include <comms/posix.hpp>

static const std::string kPortName = "/dev/ttyUSB1";
static const std::string kTestData = "spikes.txt";
static const bool kDebug = false;

static auto proc_ = neuro::Processor(2, 2, 2, 1, kDebug);

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

static auto loaddata() -> std::vector<Entry>
{
    std::ifstream file(kTestData);

    std::vector<Entry> data;             

    if (!file.is_open()) {
        std::cerr << "Error opening file " << kTestData << std::endl;
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
    proc_.Connect();

    const auto data = loaddata(); 

    const auto runtime = data.back().step + 1;

    for (int timestep=0; timestep<runtime; ++timestep) {

        proc_.ClearActivity();

        for (auto entry:data) {
            if (entry.step == timestep) {
                proc_.ApplySpike(entry.id, entry.time, entry.value);
            }
        }

        proc_.Run(50);

        printf("%03d: %02d %02d\n",
                timestep, proc_.GetOutputCount(0), proc_.GetOutputCount(1));

        if (kDebug) {
            printf("\n");
        }
    }

    return 0;
}
