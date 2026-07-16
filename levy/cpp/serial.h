/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

class Serial {

    public:
        
        Serial() = default;

        Serial(const std::string port_name, const int baud_rate) 
            : port_name_(port_name), baud_rate_(baud_rate) {}

        Serial& operator=(const Serial& other) = default;

        void Begin(const float timeout_sec=0);

        void Close();

        void Flush();

        int GetBaudRate() const { 
            return baud_rate_; 
        }

        void Write(const void * msg, const int size);

        int Read(void * msg, const int size);

    private:

        std::string port_name_;

        int baud_rate_;

        int fd_;

}; // class Serial

