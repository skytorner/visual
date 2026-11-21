cmake_minimum_required(VERSION 3.22)
project(AVSamplerAudioTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(visual_audio_tests AudioSignalAnalysisTests.cpp)
add_test(NAME visual_audio_tests COMMAND visual_audio_tests)
