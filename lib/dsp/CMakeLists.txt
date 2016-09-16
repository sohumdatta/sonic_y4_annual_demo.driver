add_library(dsp SHARED iir.c)
target_include_directories(dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
