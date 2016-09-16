add_library(emg_driver SHARED emg_driver.c)
target_include_directories(emg_driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
