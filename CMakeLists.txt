# CMakeList.txt: CMake-Projektdatei der obersten Ebene. Führen Sie hier die globale Konfiguration aus,
# und schließen Sie Unterprojekte ein.
#
cmake_minimum_required (VERSION 3.8)

project (BioSimulation)

include_directories("dependencies/CImg")
include_directories("dependencies/inih")

# Schließen Sie Unterprojekte ein.
add_subdirectory ("BioSimulation")
add_subdirectory ("dependencies/inih")
