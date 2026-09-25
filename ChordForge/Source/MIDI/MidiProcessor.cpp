#include "MidiProcessor.h"
#include <algorithm>

namespace ChordForge::Midi
{

MidiProcessor::MidiProcessor()
{
    mActiveTriggers.reserve(16);
}

void MidiProcessor::reset()
{
    mActiveTriggers.clear();
}

void MidiProcessor::processMidiInput(
    const juce::MidiBuffer& inputMidi,
    juce::MidiBuffer& outputMidi,
    int midiChannel)
{
    if (mTriggerMode == TriggerMode::Off)
    {
        // Pass through or ignore based on config
        for (const auto metadata : inputMidi)
        {
            outputMidi.addEvent(metadata.getMessage(), metadata.samplePosition);
        }
        return;
    }

    for (const auto metadata : inputMidi)
    {
        const auto msg = metadata.getMessage();
        const int sampleOffset = metadata.samplePosition;

        if (msg.isNoteOn())
        {
            handleNoteOn(msg.getNoteNumber(), msg.getVelocity(), sampleOffset, outputMidi, midiChannel);
        }
        else if (msg.isNoteOff())
        {
            handleNoteOff(msg.getNoteNumber(), sampleOffset, outputMidi, midiChannel);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            for (const auto& trig : mActiveTriggers)
            {
                for (int outNote : trig.generatedNotes)
                {
                    outputMidi.addEvent(juce::MidiMessage::noteOff(midiChannel, outNote, (uint8_t)0), sampleOffset);
                }
            }
            mActiveTriggers.clear();
        }
    }
}

void MidiProcessor::handleNoteOn(
    int inputNote,
    uint8_t velocity,
    int sampleOffset,
    juce::MidiBuffer& output,
    int channel)
{
    std::vector<Music::Note> voicedNotes;

    switch (mTriggerMode)
    {
        case TriggerMode::RootTrigger:
        {
            // Input note's pitch class becomes root; transpose to octave 4
            Music::Note in(inputNote);
            Music::Chord chord(in.getPitchClass(), mCurrentQuality, in.getOctave());
            voicedNotes = Music::Voicing::generate(chord, mVoicingOptions);
            break;
        }
        case TriggerMode::ScaleDegreeTrigger:
        {
            // White keys relative to C (0..6) map to degrees 1..7
            const int pc = inputNote % 12;
            int degree = 1;
            switch (pc)
            {
                case 0: degree = 1; break; // C
                case 2: degree = 2; break; // D
                case 4: degree = 3; break; // E
                case 5: degree = 4; break; // F
                case 7: degree = 5; break; // G
                case 9: degree = 6; break; // A
                case 11: degree = 7; break; // B
                default: degree = 1; break; // non-diatonic fallback
            }
            Music::Chord chord = Music::Chord::fromScaleDegree(mScale, degree, false, 4);
            voicedNotes = Music::Voicing::generate(chord, mVoicingOptions);
            break;
        }
        case TriggerMode::ProgressionSlotTrigger:
        {
            // Input notes C3 (48), C#3 (49), D3 (50)... trigger progression slot indices
            int slotIdx = inputNote % 12;
            const auto* slot = mProgression.getSlot(slotIdx);
            if (slot != nullptr)
            {
                voicedNotes = Music::Voicing::generate(slot->chord, slot->voicingOptions);
            }
            break;
        }
        case TriggerMode::Off:
            return;
    }

    if (voicedNotes.empty()) return;

    ActiveTrigger trigger;
    trigger.inputNoteNumber = inputNote;
    trigger.generatedNotes.reserve(voicedNotes.size());

    for (const auto& vn : voicedNotes)
    {
        int p = vn.getMidiNumber();
        output.addEvent(juce::MidiMessage::noteOn(channel, p, velocity), sampleOffset);
        trigger.generatedNotes.push_back(p);
    }

    mActiveTriggers.push_back(std::move(trigger));
}

void MidiProcessor::handleNoteOff(
    int inputNote,
    int sampleOffset,
    juce::MidiBuffer& output,
    int channel)
{
    auto it = std::find_if(mActiveTriggers.begin(), mActiveTriggers.end(),
        [inputNote](const ActiveTrigger& t) { return t.inputNoteNumber == inputNote; });

    if (it != mActiveTriggers.end())
    {
        for (int outNote : it->generatedNotes)
        {
            output.addEvent(juce::MidiMessage::noteOff(channel, outNote, (uint8_t)0), sampleOffset);
        }
        mActiveTriggers.erase(it);
    }
}

} // namespace ChordForge::Midi
