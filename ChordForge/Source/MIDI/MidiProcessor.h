#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Music/Scale.h"
#include "../Music/Chord.h"
#include "../Music/Voicing.h"
#include "../Music/Progression.h"
#include <vector>
#include <cstdint>

namespace ChordForge::Midi
{

enum class TriggerMode : uint8_t
{
    Off = 0,               // Normal DAW host sequencer playback
    RootTrigger,          // Input note defines root, plays current chord quality & voicing
    ScaleDegreeTrigger,   // Input white keys (C=I, D=ii, E=iii...) trigger diatonic chord
    ProgressionSlotTrigger // Input notes (C3..B3) trigger progression slots 0..N
};

class MidiProcessor
{
public:
    MidiProcessor();

    void setTriggerMode(TriggerMode mode) noexcept { mTriggerMode = mode; }
    [[nodiscard]] TriggerMode getTriggerMode() const noexcept { return mTriggerMode; }

    void setScale(const Music::Scale& scale) { mScale = scale; }
    void setCurrentChordQuality(Music::ChordQuality quality) { mCurrentQuality = quality; }
    void setVoicingOptions(const Music::VoicingOptions& options) { mVoicingOptions = options; }
    void setProgression(const Music::Progression& progression) { mProgression = progression; }

    // Intercept input MIDI buffer and generate output chords based on trigger mode
    void processMidiInput(
        const juce::MidiBuffer& inputMidi,
        juce::MidiBuffer& outputMidi,
        int midiChannel = 1
    );

    void reset();

private:
    TriggerMode mTriggerMode{TriggerMode::Off};
    Music::Scale mScale;
    Music::ChordQuality mCurrentQuality{Music::ChordQuality::Major};
    Music::VoicingOptions mVoicingOptions;
    Music::Progression mProgression;

    // Track active output notes generated per input note
    struct ActiveTrigger
    {
        int inputNoteNumber{-1};
        std::vector<int> generatedNotes;
    };
    std::vector<ActiveTrigger> mActiveTriggers;

    void handleNoteOn(int inputNote, uint8_t velocity, int sampleOffset, juce::MidiBuffer& output, int channel);
    void handleNoteOff(int inputNote, int sampleOffset, juce::MidiBuffer& output, int channel);
};

} // namespace ChordForge::Midi
