#include "MidiGenerator.h"
#include <cmath>

namespace ChordForge::Midi
{

MidiGenerator::MidiGenerator()
{
    mActiveNotes.reserve(MAX_SIMULTANEOUS_NOTES);
}

void MidiGenerator::reset()
{
    mCurrentSlotIndex = -1;
    mLastPpq = -1.0;
    mActiveNotes.clear();
}

void MidiGenerator::setProgression(const Music::Progression& progression)
{
    mProgression = progression;
    mCachedVoicings = mProgression.generateAllVoicings();
}

void MidiGenerator::processBlock(
    juce::MidiBuffer& midiMessages,
    int numSamples,
    double sampleRate,
    const PlaybackPosition& position)
{
    juce::ignoreUnused(numSamples, sampleRate);

    if (!position.isPlaying || mCachedVoicings.empty())
    {
        if (!mActiveNotes.empty())
        {
            sendNoteOffsForActive(midiMessages, 0);
        }
        mCurrentSlotIndex = -1;
        return;
    }

    // Progression timing: calculate total beats in progression
    double totalBeats = 0.0;
    const auto& slots = mProgression.getSlots();
    for (const auto& slot : slots)
    {
        totalBeats += slot.durationBeats;
    }
    if (totalBeats <= 0.0) totalBeats = 16.0;

    // Current beat wrapped in progression loop
    double currentBeat = std::fmod(position.ppqPosition, totalBeats);
    if (currentBeat < 0.0) currentBeat += totalBeats;

    // Determine which slot corresponds to currentBeat
    double accumulatedBeats = 0.0;
    int determinedSlot = 0;
    for (size_t i = 0; i < slots.size(); ++i)
    {
        double nextBoundary = accumulatedBeats + slots[i].durationBeats;
        if (currentBeat >= accumulatedBeats && currentBeat < nextBoundary)
        {
            determinedSlot = static_cast<int>(i);
            break;
        }
        accumulatedBeats = nextBoundary;
    }

    // If slot changed, trigger note offs for old slot and note ons for new slot
    if (determinedSlot != mCurrentSlotIndex)
    {
        sendNoteOffsForActive(midiMessages, 0);
        mCurrentSlotIndex = determinedSlot;

        if (determinedSlot >= 0 && static_cast<size_t>(determinedSlot) < mCachedVoicings.size())
        {
            const auto& notes = mCachedVoicings[static_cast<size_t>(determinedSlot)];
            for (const auto& n : notes)
            {
                int midiNum = n.getMidiNumber();
                midiMessages.addEvent(juce::MidiMessage::noteOn(mMidiChannel, midiNum, (uint8_t)100), 0);
                mActiveNotes.push_back(midiNum);
            }
        }
    }

    mLastPpq = position.ppqPosition;
}

void MidiGenerator::triggerChordNotes(
    juce::MidiBuffer& targetBuffer,
    const std::vector<Music::Note>& notes,
    uint8_t velocity,
    int sampleOffset,
    int midiChannel)
{
    sendNoteOffsForActive(targetBuffer, sampleOffset);
    mMidiChannel = midiChannel;

    for (const auto& n : notes)
    {
        int p = n.getMidiNumber();
        targetBuffer.addEvent(juce::MidiMessage::noteOn(mMidiChannel, p, velocity), sampleOffset);
        mActiveNotes.push_back(p);
    }
}

void MidiGenerator::releaseAllNotes(juce::MidiBuffer& targetBuffer, int sampleOffset, int midiChannel)
{
    mMidiChannel = midiChannel;
    sendNoteOffsForActive(targetBuffer, sampleOffset);
}

void MidiGenerator::sendNoteOffsForActive(juce::MidiBuffer& target, int sampleOffset)
{
    for (int noteNum : mActiveNotes)
    {
        target.addEvent(juce::MidiMessage::noteOff(mMidiChannel, noteNum, (uint8_t)0), sampleOffset);
    }
    mActiveNotes.clear();
}

} // namespace ChordForge::Midi
