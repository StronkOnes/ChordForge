cmake_minimum_required(VERSION 3.22)

add_executable(ChordForgeTests
    ../Source/Tests/MusicTheoryTests.cpp
    ../Source/Music/Note.cpp
    ../Source/Music/Scale.cpp
    ../Source/Music/Chord.cpp
    ../Source/Music/Voicing.cpp
    ../Source/Music/VoiceLeading.cpp
    ../Source/Music/Progression.cpp
    ../Source/MIDI/MidiGenerator.cpp
    ../Source/MIDI/MidiProcessor.cpp
    ../Source/MIDI/MidiExporter.cpp
    ../Source/Assembly/OptimizedMath.cpp
    ../Source/Presets/PresetManager.cpp
)

target_include_directories(ChordForgeTests PRIVATE
    ../Source
    ../Source/Music
    ../Source/MIDI
    ../Source/Assembly
    ../Source/Presets
    ../Source/Tests
)

target_compile_definitions(ChordForgeTests PRIVATE
    CHORDFORGE_STANDALONE_TESTS=1
)

target_link_libraries(ChordForgeTests PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_audio_basics
)

enable_testing()
add_test(NAME MusicTheoryAndMidiTests COMMAND ChordForgeTests)
