#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Music/Progression.h"
#include <vector>
#include <array>
#include <cstdint>

namespace ChordForge::Midi
{

struct PlaybackPosition
{
    double bpm{120.0};
    int64_t timeInSamples{0};
    double ppqPosition{0.0};
    bool isPlaying{false};
    int timeSigNumerator{4};
    int timeSigDenominator{4};
};

class MidiGenerator
{
public:
    MidiGenerator();

    void reset();

    // Set progression to play
    void setProgression(const Music::Progression& progression);

    // Audio/MIDI real-time processing callback
    // Real-time safe: strictly no dynamic allocations, locks, or I/O
    void processBlock(
        juce::MidiBuffer& midiMessages,
        int numSamples,
        double sampleRate,
        const PlaybackPosition& position
    );

    // Send chord notes manually (for UI auditioning or manual trigger)
    void triggerChordNotes(
        juce::MidiBuffer& targetBuffer,
        const std::vector<Music::Note>& notes,
        uint8_t velocity = 100,
        int sampleOffset = 0,
        int midiChannel = 1
    );

    void releaseAllNotes(juce::MidiBuffer& targetBuffer, int sampleOffset = 0, int midiChannel = 1);

    [[nodiscard]] int getActiveSlotIndex() const noexcept { return mCurrentSlotIndex; }
    [[nodiscard]] const std::vector<int>& getActiveNoteNumbers() const noexcept { return mActiveNotes; }

private:
    static constexpr size_t MAX_SIMULTANEOUS_NOTES = 32;

    Music::Progression mProgression;
    std::vector<std::vector<Music::Note>> mCachedVoicings;

    int mCurrentSlotIndex{-1};
    double mLastPpq{-1.0};
    std::vector<int> mActiveNotes;
    int mMidiChannel{1};

    void sendNoteOffsForActive(juce::MidiBuffer& target, int sampleOffset);
};

} // namespace ChordForge::Midi
