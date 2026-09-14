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

//#include <processor.hpp>
//#include "dronepong_fpga.hpp"

static const std::string kPortName = "/dev/ttyUSB1";
static const std::string kTestData = "spikes.txt";

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

extern void clear_encoded_spikes();
extern void encode();
extern unsigned int num_encoded_spikes;
extern void apply_spike(unsigned int input_ind, unsigned int time, double value);
extern void run(double duration);
static const unsigned int SIM_TIME = 50;
extern unsigned int output_count(unsigned int output_ind);

int main()
{
    //proc_.Connect();

    const auto data = loaddata(); 

    const auto runtime = data.back().step + 1;

    for (int timestep=0; timestep<runtime; ++timestep) {

        clear_encoded_spikes();
        encode();

        //proc_.ClearActivity();

        for (auto entry:data) {
            if (entry.step == timestep) {
                //proc_.ApplySpike(entry.id, entry.time, entry.value);
                apply_spike(entry.id, entry.time, entry.value);
            }
        }

        run(SIM_TIME);

        printf("%u %u\n", output_count(0), output_count(1));

        /*
        proc_.Run(50);

        printf("%03d: %02d %02d\n",
                timestep, proc_.GetOutputCount(0), proc_.GetOutputCount(1));
                */
    }

    return 0;
}
