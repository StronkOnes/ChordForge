#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace ChordForge
{

class ChordForgeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit ChordForgeAudioProcessorEditor(ChordForgeAudioProcessor&);
    ~ChordForgeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    ChordForgeAudioProcessor& mProcessor;

    // Custom Hardware Dark Palette LookAndFeel
    class HardwareLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        HardwareLookAndFeel();
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH,
                          juce::ComboBox& box) override;
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
    } mCustomLookAndFeel;

    // Header Controls
    juce::Label mTitleLabel;
    juce::ComboBox mPresetBox;
    juce::TextButton mSettingsButton;

    // Tone Matrix / Selectors
    juce::Label mKeyLabel;
    juce::ComboBox mKeyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mKeyAttachment;

    juce::Label mScaleLabel;
    juce::ComboBox mScaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mScaleAttachment;

    juce::Label mChordDegreeLabel;
    juce::ComboBox mChordDegreeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mChordDegreeAttachment;

    juce::ToggleButton mInclude7thToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mInclude7thAttachment;

    // Voicing Controls
    juce::Label mVoicingLabel;
    juce::ComboBox mVoicingBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mVoicingAttachment;

    juce::Label mInversionLabel;
    juce::ComboBox mInversionBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mInversionAttachment;

    juce::Label mOctaveLabel;
    juce::ComboBox mOctaveBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mOctaveAttachment;

    // Chord Info Display
    juce::Label mChordNameDisplay;
    juce::Label mNotesDisplay;

    // Progression Builder Slots
    static constexpr int MAX_DISPLAY_SLOTS = 8;
    std::array<juce::TextButton, MAX_DISPLAY_SLOTS> mSlotButtons;
    std::array<juce::ToggleButton, MAX_DISPLAY_SLOTS> mSlotLockButtons;

    // Progression Buttons
    juce::TextButton mAddChordButton;
    juce::TextButton mRandomizeButton;
    juce::TextButton mAuditionChordButton;
    juce::TextButton mExportMidiButton;
    juce::ToggleButton mVoiceLeadingToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mVoiceLeadingAttachment;

    // Trigger Mode & Transport
    juce::Label mTriggerLabel;
    juce::ComboBox mTriggerBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mTriggerAttachment;

    juce::Label mBpmLabel;
    juce::Slider mBpmSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mBpmAttachment;

    // Internal UI helpers
    void updateChordDisplay();
    void updateProgressionSlotUI();
    void paintPianoKeyboard(juce::Graphics& g, juce::Rectangle<int> bounds);

    std::vector<int> mCurrentActiveNotes;
    int mActiveSlot{-1};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordForgeAudioProcessorEditor)
};

} // namespace ChordForge
