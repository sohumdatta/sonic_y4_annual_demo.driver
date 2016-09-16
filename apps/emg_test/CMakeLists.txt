add_executable(emg_test emg_test.c)
target_link_libraries(emg_test LINK_PUBLIC emg_driver dsp)
