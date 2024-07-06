/*
 * File name: CLIHandler.h
 * Author: ToshibaMastru
 * Copyright (c) 2024 ToshibaMastru
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include "VideoPlayer.h"
#include <sstream>
#include <iostream>
#include <sstream>
#include <string>

class Logger {
public:
    enum class Level {
        Fatal,
        Info,
        Error,
        Warning,
        Log,
    };

    Logger(Level level, const std::string& element)
            : level(level), element(element) {}

    ~Logger() {
        std::string prefix;
        switch (level) {
            case Level::Fatal:   prefix = "FATAL"; break;
            case Level::Info:    prefix = "Info"; break;
            case Level::Error:   prefix = "ERROR"; break;
            case Level::Warning: prefix = "Warning"; break;
            case Level::Log:     prefix = "Log"; break;
        }

        std::cerr << "[" << element << "] " << prefix << ": " << stream.str() << std::endl;
    }

    template<typename T>
    Logger& operator<<(const T& data) {
        stream << data;
        return *this;
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        stream << manip;
        return *this;
    }

private:
    Level level;
    std::string element;
    std::ostringstream stream;
};

class CLIHandler {
public:
    static bool initialize(int debuglevel);
    static bool splitArgs(const int argc, char *argv[], VideoSettings& settings);
    static void printHelp(std::string& prog_name);
    static void printHelp(const char* prog_name);

    static Logger logger(Logger::Level level, const std::string& context) {
        return Logger(Logger::Level::Info, context);
    }
};
