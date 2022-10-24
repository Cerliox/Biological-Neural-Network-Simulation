cmake_minimum_required (VERSION 3.8)

target_sources(BioSimulation PRIVATE "${CMAKE_SOURCE_DIR}/dependencies/inih/ini.c" PRIVATE "${CMAKE_SOURCE_DIR}/dependencies/inih/INIReader.cpp")