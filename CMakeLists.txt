cmake_minimum_required(VERSION 3.22)

project(AudioReactiveImagePOC VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(JUCE_GIT_TAG "8.0.8" CACHE STRING "JUCE git tag or branch to fetch")

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG ${JUCE_GIT_TAG}
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

set(AUDIO_REACTIVE_IMAGE_FORMATS VST3 Standalone)

if(APPLE)
    list(APPEND AUDIO_REACTIVE_IMAGE_FORMATS AU)
endif()

juce_add_plugin(AudioReactiveImagePOC
    COMPANY_NAME "Visual POC"
    BUNDLE_ID "com.visual.audioreactiveimagepoc"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Vpoc
    PLUGIN_CODE Arip
    FORMATS ${AUDIO_REACTIVE_IMAGE_FORMATS}
    PRODUCT_NAME "AudioReactiveImagePOC"
)

juce_generate_juce_header(AudioReactiveImagePOC)

target_sources(AudioReactiveImagePOC
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_features(AudioReactiveImagePOC PRIVATE cxx_std_17)

target_compile_definitions(AudioReactiveImagePOC
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AudioReactiveImagePOC
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
