#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Music/Progression.h"
#include <string>
#include <vector>

namespace ChordForge::Midi
{

class MidiExporter
{
public:
    // Generate standard MIDI File (Type 1) containing all chords in progression
    static juce::MidiFile createMidiFile(
        const Music::Progression& progression,
        int ppqResolution = 960,
        int midiChannel = 1,
        uint8_t velocity = 96
    );

    // Write standard MIDI file to disk (.mid)
    static bool exportToFile(
        const Music::Progression& progression,
        const std::string& destinationFilePath,
        int ppqResolution = 960,
        int midiChannel = 1
    );

    // Get raw Standard MIDI file bytes for drag-and-drop or web download
    static std::vector<uint8_t> getMidiFileBytes(
        const Music::Progression& progression,
        int ppqResolution = 960,
        int midiChannel = 1
    );
};

} // namespace ChordForge::Midi
