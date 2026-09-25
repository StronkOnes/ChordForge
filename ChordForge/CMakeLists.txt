cmake_minimum_required(VERSION 3.22)
project(ChordForge VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JUCE integration via FetchContent if not provided externally
include(FetchContent)

if(NOT TARGET juce::juce_audio_utils)
    message(STATUS "Fetching JUCE 7.0.9...")
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.9
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Plugin Target Configuration
juce_add_plugin(ChordForge
    COMPANY_NAME "ChordForge Audio"
    PLUGIN_MANUFACTURER_CODE "ChdF"
    PLUGIN_CODE "CFrg"
    FORMATS VST3 Standalone
    PRODUCT_NAME "ChordForge"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Source files
target_sources(ChordForge PRIVATE
    # Plugin layer
    Source/Plugin/PluginProcessor.cpp
    Source/Plugin/PluginProcessor.h
    Source/Plugin/PluginEditor.cpp
    Source/Plugin/PluginEditor.h

    # Music Theory Engine
    Source/Music/Note.cpp
    Source/Music/Note.h
    Source/Music/Scale.cpp
    Source/Music/Scale.h
    Source/Music/Chord.cpp
    Source/Music/Chord.h
    Source/Music/Voicing.cpp
    Source/Music/Voicing.h
    Source/Music/VoiceLeading.cpp
    Source/Music/VoiceLeading.h
    Source/Music/Progression.cpp
    Source/Music/Progression.h

    # MIDI Engine
    Source/MIDI/MidiGenerator.cpp
    Source/MIDI/MidiGenerator.h
    Source/MIDI/MidiProcessor.cpp
    Source/MIDI/MidiProcessor.h
    Source/MIDI/MidiExporter.cpp
    Source/MIDI/MidiExporter.h

    # Optimized Math / Assembly abstraction
    Source/Assembly/OptimizedMath.cpp
    Source/Assembly/OptimizedMath.h

    # Presets
    Source/Presets/PresetManager.cpp
    Source/Presets/PresetManager.h
)

target_include_directories(ChordForge PRIVATE
    Source
    Source/Music
    Source/MIDI
    Source/Plugin
    Source/Assembly
    Source/Presets
)

target_compile_definitions(ChordForge PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    CHORDFORGE_VERSION="1.0.0"
)

target_link_libraries(ChordForge PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_data_structures
    juce::juce_events
    juce::juce_core
)

juce_generate_juce_header(ChordForge)

# Enable Tests Option
option(BUILD_CHORDFORGE_TESTS "Build ChordForge automated unit tests" ON)

if(BUILD_CHORDFORGE_TESTS)
    add_subdirectory(tests)
endif()
