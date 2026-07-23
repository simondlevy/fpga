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
static constexpr int kSpikeValueFactor = 10;

typedef struct {

    int id;
    float time;
    float value;
} Spike; 


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
    /*
       auto conn = neuro::FpgaConnection(
       kNumInputs,
            kNumOutputs,
            kChargeWidth,
            kClockFreq,
            kSpikeValueFactor);
            */


    std::ifstream file("../spikes.txt"); 

    std::vector<Spike> data;             


    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
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

        std::cout << step << " " << id << " " << time << " " << value << std::endl;

    }

    file.close();

    return 0;

    /*
    for (uint8_t k=0; k<2; ++k) {

        conn.ClearActivity();

        for (int i=0; i<28; ++i) {
            conn.ApplySpike(0, i, 1.0);
        }

        for (int i=0; i<26; ++i) {
            conn.ApplySpike(1, i, 1.0);
        }

        conn.Run(50);

        printf("%d %d\n\n", conn.GetOutputCount(0), conn.GetOutputCount(1));
    }*/

    file.close(); 

    return 0;
}
