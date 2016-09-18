add_executable(collect_data collect_data.c)
target_link_libraries(collect_data LINK_PUBLIC emg_driver dsp)
