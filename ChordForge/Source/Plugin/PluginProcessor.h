#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Music/Scale.h"
#include "../Music/Chord.h"
#include "../Music/Voicing.h"
#include "../Music/Progression.h"
#include "../MIDI/MidiGenerator.h"
#include "../MIDI/MidiProcessor.h"
#include "../Presets/PresetManager.h"

namespace ChordForge
{

class ChordForgeAudioProcessor : public juce::AudioProcessor
{
public:
    ChordForgeAudioProcessor();
    ~ChordForgeAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ChordForge"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return mParameters; }

    // Authoritative Progression & State
    Music::Progression& getProgression() noexcept { return mProgression; }
    const Music::Progression& getProgression() const noexcept { return mProgression; }
    void updateProgressionFromParameters();

    // Preset Manager
    Presets::PresetManager& getPresetManager() noexcept { return mPresetManager; }
    void loadPreset(const Presets::Preset& preset);

    // Active visual state for UI queries
    [[nodiscard]] std::vector<int> getActiveMidiNotes() const;
    [[nodiscard]] int getActiveProgressionSlot() const noexcept;

    // UI Trigger request (thread-safe queue)
    void requestTriggerAudition(const std::vector<Music::Note>& notes);
    void requestReleaseAudition();

private:
    juce::AudioProcessorValueTreeState mParameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Music::Scale mCurrentScale;
    Music::Progression mProgression;
    Midi::MidiGenerator mMidiGenerator;
    Midi::MidiProcessor mMidiProcessor;
    Presets::PresetManager mPresetManager;

    // Thread-safe lock-free flags for UI auditioning
    std::atomic<bool> mAuditionTriggerRequested{false};
    std::atomic<bool> mAuditionReleaseRequested{false};
    std::vector<Music::Note> mAuditionNotes;
    juce::CriticalSection mAuditionLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordForgeAudioProcessor)
};

} // namespace ChordForge
