#include "PluginEditor.h"
#include "../MIDI/MidiExporter.h"

namespace ChordForge
{

ChordForgeAudioProcessorEditor::HardwareLookAndFeel::HardwareLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff121417));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e2126));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffdcdfe4));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2d3139));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8a909d));
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22262e));
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe0e4eb));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff3b82f6));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff1e2126));
}

void ChordForgeAudioProcessorEditor::HardwareLookAndFeel::drawComboBox(
    juce::Graphics& g, int width, int height, bool isButtonDown,
    int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);
    auto bounds = box.getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff1e2126));
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff2d3139));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Subtle arrow icon
    auto arrowX = width - 18.0f;
    auto arrowY = height * 0.5f - 2.0f;
    juce::Path p;
    p.addTriangle(arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour(juce::Colour(0xff8a909d));
    g.fillPath(p);
}

void ChordForgeAudioProcessorEditor::HardwareLookAndFeel::drawButtonBackground(
    juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    juce::Colour base = backgroundColour;

    if (shouldDrawButtonAsDown)
        base = base.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        base = base.brighter(0.1f);

    g.setColour(base);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff2d3139));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

ChordForgeAudioProcessorEditor::ChordForgeAudioProcessorEditor(ChordForgeAudioProcessor& p)
    : AudioProcessorEditor(&p), mProcessor(p)
{
    setLookAndFeel(&mCustomLookAndFeel);

    // Title & Branding
    mTitleLabel.setText("CHORDFORGE", juce::dontSendNotification);
    mTitleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    mTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    addAndMakeVisible(mTitleLabel);

    // Presets Box
    mPresetBox.setTextWhenNothingSelected("Select Preset...");
    const auto& presets = mProcessor.getPresetManager().getPresets();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        mPresetBox.addItem(presets[i].category + ": " + presets[i].name, static_cast<int>(i + 1));
    }
    mPresetBox.onChange = [this]() {
        int idx = mPresetBox.getSelectedId() - 1;
        if (const auto* p = mProcessor.getPresetManager().getPreset(static_cast<size_t>(idx)))
        {
            mProcessor.loadPreset(*p);
            updateChordDisplay();
            updateProgressionSlotUI();
        }
    };
    addAndMakeVisible(mPresetBox);

    // Key Selector
    mKeyLabel.setText("KEY", juce::dontSendNotification);
    mKeyLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mKeyLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mKeyLabel);

    const char* keys[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 12; ++i) mKeyBox.addItem(keys[i], i + 1);
    addAndMakeVisible(mKeyBox);
    mKeyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "key", mKeyBox);
    mKeyBox.onChange = [this]() { mProcessor.updateProgressionFromParameters(); updateChordDisplay(); };

    // Scale Selector
    mScaleLabel.setText("SCALE", juce::dontSendNotification);
    mScaleLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mScaleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mScaleLabel);

    const char* scales[] = {"Major", "Natural Minor", "Dorian", "Phrygian", "Lydian",
                            "Mixolydian", "Locrian", "Harmonic Minor", "Melodic Minor"};
    for (int i = 0; i < 9; ++i) mScaleBox.addItem(scales[i], i + 1);
    addAndMakeVisible(mScaleBox);
    mScaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "scale", mScaleBox);
    mScaleBox.onChange = [this]() { mProcessor.updateProgressionFromParameters(); updateChordDisplay(); };

    // Diatonic Degree Selector
    mChordDegreeLabel.setText("DEGREE", juce::dontSendNotification);
    mChordDegreeLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mChordDegreeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mChordDegreeLabel);

    const char* degrees[] = {"I", "II", "III", "IV", "V", "VI", "VII"};
    for (int i = 0; i < 7; ++i) mChordDegreeBox.addItem(degrees[i], i + 1);
    addAndMakeVisible(mChordDegreeBox);
    mChordDegreeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "chord_degree", mChordDegreeBox);
    mChordDegreeBox.onChange = [this]() { updateChordDisplay(); };

    // 7th Toggle
    mInclude7thToggle.setButtonText("+7th");
    addAndMakeVisible(mInclude7thToggle);
    mInclude7thAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        mProcessor.getAPVTS(), "include_7th", mInclude7thToggle);
    mInclude7thToggle.onClick = [this]() { updateChordDisplay(); };

    // Voicing Style
    mVoicingLabel.setText("VOICING", juce::dontSendNotification);
    mVoicingLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mVoicingLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mVoicingLabel);

    const char* voicings[] = {"Close", "Open", "Wide", "Drop 2", "Piano", "Guitar", "Pad", "Bass + Chord"};
    for (int i = 0; i < 8; ++i) mVoicingBox.addItem(voicings[i], i + 1);
    addAndMakeVisible(mVoicingBox);
    mVoicingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "voicing", mVoicingBox);
    mVoicingBox.onChange = [this]() { updateChordDisplay(); };

    // Inversion
    mInversionLabel.setText("INV", juce::dontSendNotification);
    mInversionLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mInversionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mInversionLabel);

    for (int i = 0; i < 4; ++i) mInversionBox.addItem(std::to_string(i), i + 1);
    addAndMakeVisible(mInversionBox);
    mInversionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "inversion", mInversionBox);
    mInversionBox.onChange = [this]() { updateChordDisplay(); };

    // Octave
    mOctaveLabel.setText("OCT", juce::dontSendNotification);
    mOctaveLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mOctaveLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mOctaveLabel);

    const char* octs[] = {"-2", "-1", "0", "+1", "+2"};
    for (int i = 0; i < 5; ++i) mOctaveBox.addItem(octs[i], i + 1);
    addAndMakeVisible(mOctaveBox);
    mOctaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "octave", mOctaveBox);
    mOctaveBox.onChange = [this]() { updateChordDisplay(); };

    // Display Labels
    mChordNameDisplay.setFont(juce::Font(22.0f, juce::Font::bold));
    mChordNameDisplay.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    addAndMakeVisible(mChordNameDisplay);

    mNotesDisplay.setFont(juce::Font(13.0f, juce::Font::plain));
    mNotesDisplay.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(mNotesDisplay);

    // Audition Button (Plays current chord immediately)
    mAuditionChordButton.setButtonText("▶ Audition");
    mAuditionChordButton.onClick = [this]() {
        int key = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("key"));
        int scale = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("scale"));
        int deg = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("chord_degree"));
        bool has7th = *mProcessor.getAPVTS().getRawParameterValue("include_7th") > 0.5f;

        Music::Scale sc(static_cast<Music::PitchClass>(key), static_cast<Music::ScaleType>(scale));
        Music::Chord chord = Music::Chord::fromScaleDegree(sc, deg, has7th, 4);

        Music::VoicingOptions opts;
        opts.style = static_cast<Music::VoicingStyle>(static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("voicing")));
        opts.inversion = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("inversion"));
        opts.octaveShift = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("octave")) - 2;

        auto voicedNotes = Music::Voicing::generate(chord, opts);
        mProcessor.requestTriggerAudition(voicedNotes);
    };
    addAndMakeVisible(mAuditionChordButton);

    // Progression Slot Buttons
    for (int i = 0; i < MAX_DISPLAY_SLOTS; ++i)
    {
        mSlotButtons[i].setButtonText(juce::String(i + 1));
        mSlotButtons[i].onClick = [this, i]() {
            if (i < static_cast<int>(mProcessor.getProgression().getSlotCount()))
            {
                const auto* slot = mProcessor.getProgression().getSlot(i);
                if (slot != nullptr)
                {
                    auto voiced = Music::Voicing::generate(slot->chord, slot->voicingOptions);
                    mProcessor.requestTriggerAudition(voiced);
                }
            }
        };
        addAndMakeVisible(mSlotButtons[i]);

        mSlotLockButtons[i].setButtonText("🔒");
        mSlotLockButtons[i].onClick = [this, i]() {
            bool locked = mSlotLockButtons[i].getToggleState();
            mProcessor.getProgression().setSlotLocked(i, locked);
        };
        addAndMakeVisible(mSlotLockButtons[i]);
    }

    // Add Chord Button
    mAddChordButton.setButtonText("+ Add Chord");
    mAddChordButton.onClick = [this]() {
        int key = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("key"));
        int scale = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("scale"));
        int deg = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("chord_degree"));
        bool has7th = *mProcessor.getAPVTS().getRawParameterValue("include_7th") > 0.5f;

        Music::Scale sc(static_cast<Music::PitchClass>(key), static_cast<Music::ScaleType>(scale));
        Music::ProgressionSlot slot;
        slot.chord = Music::Chord::fromScaleDegree(sc, deg, has7th, 4);
        slot.voicingOptions.style = static_cast<Music::VoicingStyle>(static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("voicing")));
        slot.durationBeats = 4.0f;

        mProcessor.getProgression().addSlot(slot);
        mProcessor.updateProgressionFromParameters();
        updateProgressionSlotUI();
    };
    addAndMakeVisible(mAddChordButton);

    // Randomize Button
    mRandomizeButton.setButtonText("🎲 Randomize");
    mRandomizeButton.onClick = [this]() {
        mProcessor.getProgression().randomize(Music::RandomizerStyle::Pop, 4);
        mProcessor.updateProgressionFromParameters();
        updateProgressionSlotUI();
    };
    addAndMakeVisible(mRandomizeButton);

    // Voice Leading Toggle
    mVoiceLeadingToggle.setButtonText("Smooth Voice Leading");
    addAndMakeVisible(mVoiceLeadingToggle);
    mVoiceLeadingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        mProcessor.getAPVTS(), "voice_leading", mVoiceLeadingToggle);

    // Export MIDI Button
    mExportMidiButton.setButtonText("Export MIDI (.mid)");
    mExportMidiButton.onClick = [this]() {
        juce::FileChooser chooser("Save Progression as MIDI...",
                                  juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("ChordForge_Progression.mid"),
                                  "*.mid");
        if (chooser.browseForFileToSave(true))
        {
            auto file = chooser.getResult();
            Midi::MidiExporter::exportToFile(mProcessor.getProgression(), file.getFullPathName().toStdString());
        }
    };
    addAndMakeVisible(mExportMidiButton);

    // Trigger Mode
    mTriggerLabel.setText("TRIGGER", juce::dontSendNotification);
    mTriggerLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mTriggerLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mTriggerLabel);

    const char* trigs[] = {"Host DAW Sequencer", "Root Trigger", "Scale Degree Trigger", "Progression Slot Trigger"};
    for (int i = 0; i < 4; ++i) mTriggerBox.addItem(trigs[i], i + 1);
    addAndMakeVisible(mTriggerBox);
    mTriggerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        mProcessor.getAPVTS(), "trigger_mode", mTriggerBox);

    // BPM Slider
    mBpmLabel.setText("BPM", juce::dontSendNotification);
    mBpmLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    mBpmLabel.setColour(juce::Label::textColourId, juce::Colour(0xff717684));
    addAndMakeVisible(mBpmLabel);

    mBpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mBpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(mBpmSlider);
    mBpmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        mProcessor.getAPVTS(), "bpm", mBpmSlider);

    setSize(720, 480);
    updateChordDisplay();
    updateProgressionSlotUI();

    startTimerHz(25); // Refresh UI at 25 Hz for visual feedback of active notes
}

ChordForgeAudioProcessorEditor::~ChordForgeAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ChordForgeAudioProcessorEditor::timerCallback()
{
    auto notes = mProcessor.getActiveMidiNotes();
    int slot = mProcessor.getActiveProgressionSlot();

    if (notes != mCurrentActiveNotes || slot != mActiveSlot)
    {
        mCurrentActiveNotes = notes;
        mActiveSlot = slot;
        repaint();
    }
}

void ChordForgeAudioProcessorEditor::updateChordDisplay()
{
    int key = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("key"));
    int scale = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("scale"));
    int deg = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("chord_degree"));
    bool has7th = *mProcessor.getAPVTS().getRawParameterValue("include_7th") > 0.5f;

    Music::Scale sc(static_cast<Music::PitchClass>(key), static_cast<Music::ScaleType>(scale));
    Music::Chord chord = Music::Chord::fromScaleDegree(sc, deg, has7th, 4);

    Music::VoicingOptions opts;
    opts.style = static_cast<Music::VoicingStyle>(static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("voicing")));
    opts.inversion = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("inversion"));
    opts.octaveShift = static_cast<int>(*mProcessor.getAPVTS().getRawParameterValue("octave")) - 2;

    auto voicedNotes = Music::Voicing::generate(chord, opts);

    mChordNameDisplay.setText(chord.getRomanNumeral() + "  " + chord.getName(), juce::dontSendNotification);

    std::string notesStr = "Notes: ";
    for (size_t i = 0; i < voicedNotes.size(); ++i)
    {
        notesStr += voicedNotes[i].getFullName();
        if (i + 1 < voicedNotes.size()) notesStr += " · ";
    }
    mNotesDisplay.setText(notesStr, juce::dontSendNotification);
}

void ChordForgeAudioProcessorEditor::updateProgressionSlotUI()
{
    const auto& slots = mProcessor.getProgression().getSlots();
    for (int i = 0; i < MAX_DISPLAY_SLOTS; ++i)
    {
        if (i < static_cast<int>(slots.size()))
        {
            mSlotButtons[i].setVisible(true);
            mSlotLockButtons[i].setVisible(true);
            mSlotButtons[i].setButtonText(slots[i].getDisplayLabel());
            mSlotLockButtons[i].setToggleState(slots[i].isLocked, juce::dontSendNotification);
        }
        else
        {
            mSlotButtons[i].setVisible(false);
            mSlotLockButtons[i].setVisible(false);
        }
    }
}

void ChordForgeAudioProcessorEditor::paintPianoKeyboard(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Render 2-octave mini piano keyboard C3 (48) to B4 (71)
    constexpr int START_NOTE = 48;
    constexpr int NUM_NATURALS = 14; // 2 octaves of naturals
    const float whiteKeyWidth = static_cast<float>(bounds.getWidth()) / static_cast<float>(NUM_NATURALS);
    const float blackKeyWidth = whiteKeyWidth * 0.62f;
    const float blackKeyHeight = static_cast<float>(bounds.getHeight()) * 0.60f;

    // Draw white keys first
    int whiteIdx = 0;
    for (int note = START_NOTE; note < START_NOTE + 24; ++note)
    {
        int pc = note % 12;
        bool isBlack = (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10);
        if (!isBlack)
        {
            auto keyRect = juce::Rectangle<float>(bounds.getX() + whiteIdx * whiteKeyWidth,
                                                  (float)bounds.getY(), whiteKeyWidth - 1.0f, (float)bounds.getHeight());

            bool isActive = std::find(mCurrentActiveNotes.begin(), mCurrentActiveNotes.end(), note) != mCurrentActiveNotes.end();
            g.setColour(isActive ? juce::Colour(0xff38bdf8) : juce::Colour(0xffe2e8f0));
            g.fillRoundedRectangle(keyRect, 2.0f);

            whiteIdx++;
        }
    }

    // Draw black keys on top
    whiteIdx = 0;
    for (int note = START_NOTE; note < START_NOTE + 24; ++note)
    {
        int pc = note % 12;
        bool isBlack = (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10);
        if (!isBlack)
        {
            whiteIdx++;
        }
        else
        {
            float blackX = bounds.getX() + (whiteIdx * whiteKeyWidth) - (blackKeyWidth * 0.5f);
            auto blackRect = juce::Rectangle<float>(blackX, (float)bounds.getY(), blackKeyWidth, blackKeyHeight);

            bool isActive = std::find(mCurrentActiveNotes.begin(), mCurrentActiveNotes.end(), note) != mCurrentActiveNotes.end();
            g.setColour(isActive ? juce::Colour(0xff0284c7) : juce::Colour(0xff18181b));
            g.fillRoundedRectangle(blackRect, 2.0f);
        }
    }
}

void ChordForgeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0f1115));

    // Header separator line
    g.setColour(juce::Colour(0xff1f242c));
    g.drawLine(0.0f, 48.0f, (float)getWidth(), 48.0f, 1.0f);

    // Progression Section Background Box
    auto progBox = juce::Rectangle<float>(16.0f, 230.0f, (float)getWidth() - 32.0f, 120.0f);
    g.setColour(juce::Colour(0xff14171d));
    g.fillRoundedRectangle(progBox, 6.0f);
    g.setColour(juce::Colour(0xff222730));
    g.drawRoundedRectangle(progBox, 6.0f, 1.0f);

    // Section Label: "CHORD PROGRESSION"
    g.setColour(juce::Colour(0xff64748b));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("PROGRESSION BUILDER", 24, 236, 200, 14, juce::Justification::left);

    // Bottom Keyboard Visualizer
    paintPianoKeyboard(g, juce::Rectangle<int>(16, 410, getWidth() - 32, 54));
}

void ChordForgeAudioProcessorEditor::resized()
{
    // Header
    mTitleLabel.setBounds(18, 10, 160, 28);
    mPresetBox.setBounds(getWidth() - 240, 12, 220, 26);

    // Matrix Row 1: Selectors
    int y = 60;
    mKeyLabel.setBounds(20, y, 60, 14);
    mKeyBox.setBounds(20, y + 16, 68, 28);

    mScaleLabel.setBounds(100, y, 120, 14);
    mScaleBox.setBounds(100, y + 16, 130, 28);

    mChordDegreeLabel.setBounds(244, y, 70, 14);
    mChordDegreeBox.setBounds(244, y + 16, 75, 28);

    mInclude7thToggle.setBounds(328, y + 18, 65, 24);

    // Voicing controls
    mVoicingLabel.setBounds(410, y, 100, 14);
    mVoicingBox.setBounds(410, y + 16, 110, 28);

    mInversionLabel.setBounds(532, y, 50, 14);
    mInversionBox.setBounds(532, y + 16, 55, 28);

    mOctaveLabel.setBounds(600, y, 50, 14);
    mOctaveBox.setBounds(600, y + 16, 55, 28);

    // Active Chord preview row
    mChordNameDisplay.setBounds(20, 120, 300, 32);
    mNotesDisplay.setBounds(20, 152, 400, 22);
    mAuditionChordButton.setBounds(getWidth() - 150, 130, 130, 32);

    // Progression Row
    int slotX = 26;
    int slotW = 75;
    for (int i = 0; i < MAX_DISPLAY_SLOTS; ++i)
    {
        mSlotButtons[i].setBounds(slotX + (i * (slotW + 8)), 258, slotW, 44);
        mSlotLockButtons[i].setBounds(slotX + (i * (slotW + 8)) + 22, 306, 36, 20);
    }

    // Progression action buttons
    mAddChordButton.setBounds(26, 358, 100, 26);
    mRandomizeButton.setBounds(136, 358, 110, 26);
    mVoiceLeadingToggle.setBounds(260, 358, 160, 26);
    mExportMidiButton.setBounds(getWidth() - 170, 358, 150, 26);

    // Bottom controls
    mTriggerLabel.setBounds(20, 388, 70, 14);
    mTriggerBox.setBounds(85, 385, 160, 22);

    mBpmLabel.setBounds(265, 388, 35, 14);
    mBpmSlider.setBounds(300, 385, 180, 22);
}

} // namespace ChordForge
