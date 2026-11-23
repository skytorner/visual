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

# Phase 1 is macOS/AU-first. Keep the existing audio effect as a separate
# target so its plugin identity and saved projects remain compatible.
if(APPLE)
    juce_add_plugin(AVSampler
        COMPANY_NAME "Visual POC"
        BUNDLE_ID "com.visual.avsampler"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Vpoc
        PLUGIN_CODE AVsm
        VERSION 0.1.0
        FORMATS AU
        PRODUCT_NAME "AVSampler"
    )

    juce_generate_juce_header(AVSampler)

    target_sources(AVSampler
        PRIVATE
            Source/AVSamplerProcessor.cpp
            Source/AVSamplerEditor.cpp
            Source/CueTimelineComponent.cpp
            Source/SamplerUiComponents.cpp
    )

    target_compile_features(AVSampler PRIVATE cxx_std_17)
    target_compile_definitions(AVSampler
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(AVSampler
        PRIVATE
            juce::juce_audio_utils
            juce::juce_gui_extra
            juce::juce_video
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(CueRoutingTests tests/CueRoutingTests.cpp)
    target_include_directories(CueRoutingTests PRIVATE Source)
    target_compile_features(CueRoutingTests PRIVATE cxx_std_17)
    add_test(NAME CueRoutingTests COMMAND CueRoutingTests)

    add_executable(TimelineMathTests tests/TimelineMathTests.cpp)
    target_include_directories(TimelineMathTests PRIVATE Source)
    target_compile_features(TimelineMathTests PRIVATE cxx_std_17)
    add_test(NAME TimelineMathTests COMMAND TimelineMathTests)

    if(APPLE)
        add_executable(AVSamplerCoreTests tests/AVSamplerCoreTests.cpp)
        target_include_directories(AVSamplerCoreTests PRIVATE
            Source
            "${CMAKE_CURRENT_BINARY_DIR}/AVSampler_artefacts/JuceLibraryCode"
            "${JUCE_MODULES_DIR}"
        )
        target_compile_features(AVSamplerCoreTests PRIVATE cxx_std_17)
        target_compile_definitions(AVSamplerCoreTests PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
        )
        target_link_libraries(AVSamplerCoreTests PRIVATE
            AVSampler
            juce::juce_audio_utils
            juce::juce_gui_extra
            juce::juce_video
        )
        add_test(NAME AVSamplerCoreTests COMMAND AVSamplerCoreTests)
    endif()
endif()
