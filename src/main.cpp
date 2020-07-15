//
// Created by Rayan Asadi on 2020-06-28.
//

#include "gameboy.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

int main(int argc, char *argv[]) {
    // Setup logging
//    auto file_logger = spdlog::basic_logger_mt("file_logger", "log.txt", true);
//    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("%v");

    if (argc != 2) {
        std::cout << "Usage: ./GBEmu <path_to_file>" << std::endl;
    }

    std::string file_name = argv[1];
    Gameboy gameboy = Gameboy(file_name);
    gameboy.run();
}
