cmake_minimum_required(VERSION 2.8.11)
project(emg)

add_subdirectory(lib/emg_driver)
add_subdirectory(lib/dsp)
add_subdirectory(apps/emg_test)
add_subdirectory(apps/collect_data)
