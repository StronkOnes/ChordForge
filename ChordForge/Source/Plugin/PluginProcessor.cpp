#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ChordForge
{

juce::AudioProcessorValueTreeState::ParameterLayout ChordForgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Key (0..11)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"key", 1}, "Key",
        juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 0));

    // Scale Type (0..8)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"scale", 1}, "Scale",
        juce::StringArray{"Major", "Natural Minor", "Dorian", "Phrygian", "Lydian",
                          "Mixolydian", "Locrian", "Harmonic Minor", "Melodic Minor"}, 0));

    // Current Chord Degree (1..7)
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"chord_degree", 1}, "Chord Degree", 1, 7, 1));

    // 7th enable toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"include_7th", 1}, "Include 7th", false));

    // Voicing Style
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"voicing", 1}, "Voicing",
        juce::StringArray{"Close", "Open", "Wide", "Drop 2", "Piano", "Guitar", "Pad", "Bass + Chord"}, 0));

    // Inversion (0..3)
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"inversion", 1}, "Inversion", 0, 3, 0));

    // Octave Shift (-2..+2)
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"octave", 1}, "Octave Shift", -2, 2, 0));

    // Trigger Mode (0 = Off/Sequencer, 1 = Root Trigger, 2 = Scale Degree, 3 = Progression Slot)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"trigger_mode", 1}, "Trigger Mode",
        juce::StringArray{"Sequencer / Host", "Root Trigger", "Scale Degree Trigger", "Progression Slot Trigger"}, 0));

    // Voice Leading Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"voice_leading", 1}, "Voice Leading", true));

    // BPM (20..300)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bpm", 1}, "BPM", 20.0f, 300.0f, 120.0f));

    return { params.begin(), params.end() };
}

ChordForgeAudioProcessor::ChordForgeAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      mParameters(*this, nullptr, "Parameters", createParameterLayout()),
      mCurrentScale(Music::PitchClass::C, Music::ScaleType::Major),
      mProgression(mCurrentScale)
{
    mMidiGenerator.setProgression(mProgression);
    mMidiProcessor.setScale(mCurrentScale);
}

ChordForgeAudioProcessor::~ChordForgeAudioProcessor() = default;

void ChordForgeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    mMidiGenerator.reset();
    mMidiProcessor.reset();
}

void ChordForgeAudioProcessor::releaseResources()
{
    mMidiGenerator.reset();
    mMidiProcessor.reset();
}

bool ChordForgeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // ChordForge is primarily a MIDI effect/generator. It accepts any layout or MIDI only.
    if (layouts.getMainOutput() == juce::AudioChannelSet::disabled())
        return true;

    return layouts.getMainOutput() == layouts.getMainInput();
}

void ChordForgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear audio buffer (MIDI generator, transparent passthrough or silence)
    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    // 1. Process UI audition requests if pending
    if (mAuditionReleaseRequested.exchange(false))
    {
        mMidiGenerator.releaseAllNotes(midiMessages, 0);
    }

    if (mAuditionTriggerRequested.exchange(false))
    {
        std::vector<Music::Note> notesToAudition;
        {
            const juce::ScopedLock sl(mAuditionLock);
            notesToAudition = mAuditionNotes;
        }
        if (!notesToAudition.empty())
        {
            mMidiGenerator.triggerChordNotes(midiMessages, notesToAudition, 100, 0);
        }
    }

    // 2. Query Host Playhead / Transport
    Midi::PlaybackPosition pos;
    pos.bpm = *mParameters.getRawParameterValue("bpm");

    if (auto* playHead = getPlayHead())
    {
        if (auto currentPos = playHead->getPosition())
        {
            if (currentPos->getBpm().hasValue())
                pos.bpm = *currentPos->getBpm();

            if (currentPos->getTimeInSamples().hasValue())
                pos.timeInSamples = *currentPos->getTimeInSamples();

            if (currentPos->getPpqPosition().hasValue())
                pos.ppqPosition = *currentPos->getPpqPosition();

            pos.isPlaying = currentPos->getIsPlaying();

            if (currentPos->getTimeSignature().hasValue())
            {
                pos.timeSigNumerator = currentPos->getTimeSignature()->numerator;
                pos.timeSigDenominator = currentPos->getTimeSignature()->denominator;
            }
        }
    }

    // 3. Process Input MIDI if in Trigger mode
    int triggerModeIndex = static_cast<int>(*mParameters.getRawParameterValue("trigger_mode"));
    mMidiProcessor.setTriggerMode(static_cast<Midi::TriggerMode>(triggerModeIndex));

    if (triggerModeIndex > 0)
    {
        juce::MidiBuffer processedMidi;
        mMidiProcessor.processMidiInput(midiMessages, processedMidi);
        midiMessages.swapWith(processedMidi);
    }
    else
    {
        // 4. Sequencer mode: synchronize progression with DAW playhead
        mMidiGenerator.processBlock(midiMessages, buffer.getNumSamples(), getSampleRate(), pos);
    }
}

juce::AudioProcessorEditor* ChordForgeAudioProcessor::createEditor()
{
    return new ChordForgeAudioProcessorEditor(*this);
}

void ChordForgeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = mParameters.copyState();
    state.setProperty("progression_data", mProgression.getSlots().size(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ChordForgeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(mParameters.state.getType()))
    {
        mParameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        updateProgressionFromParameters();
    }
}

void ChordForgeAudioProcessor::updateProgressionFromParameters()
{
    auto keyIdx = static_cast<int>(*mParameters.getRawParameterValue("key"));
    auto scaleIdx = static_cast<int>(*mParameters.getRawParameterValue("scale"));
    bool voiceLeading = *mParameters.getRawParameterValue("voice_leading") > 0.5f;

    mCurrentScale = Music::Scale(static_cast<Music::PitchClass>(keyIdx), static_cast<Music::ScaleType>(scaleIdx));
    mProgression.setScale(mCurrentScale, true);
    mProgression.setVoiceLeadingEnabled(voiceLeading);

    mMidiGenerator.setProgression(mProgression);
    mMidiProcessor.setScale(mCurrentScale);
}

void ChordForgeAudioProcessor::loadPreset(const Presets::Preset& preset)
{
    mParameters.getParameterAsValue("key").setValue(static_cast<int>(preset.key));
    mParameters.getParameterAsValue("scale").setValue(static_cast<int>(preset.scaleType));
    mParameters.getParameterAsValue("voicing").setValue(static_cast<int>(preset.defaultVoicing));
    mParameters.getParameterAsValue("bpm").setValue(preset.bpm);

    mProgression = preset.progression;
    mMidiGenerator.setProgression(mProgression);
    mMidiProcessor.setProgression(mProgression);
}

std::vector<int> ChordForgeAudioProcessor::getActiveMidiNotes() const
{
    return mMidiGenerator.getActiveNoteNumbers();
}

int ChordForgeAudioProcessor::getActiveProgressionSlot() const noexcept
{
    return mMidiGenerator.getActiveSlotIndex();
}

void ChordForgeAudioProcessor::requestTriggerAudition(const std::vector<Music::Note>& notes)
{
    {
        const juce::ScopedLock sl(mAuditionLock);
        mAuditionNotes = notes;
    }
    mAuditionTriggerRequested.store(true);
}

void ChordForgeAudioProcessor::requestReleaseAudition()
{
    mAuditionReleaseRequested.store(true);
}

} // namespace ChordForge

// JUCE Plugin Entry Point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChordForge::ChordForgeAudioProcessor();
}
