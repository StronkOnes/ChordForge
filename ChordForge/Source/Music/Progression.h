#pragma once

#include "Chord.h"
#include "Note.h"
#include "Scale.h"
#include "Voicing.h"
#include <vector>
#include <string>
#include <cstdint>

namespace ChordForge::Music
{

struct ProgressionSlot
{
    Chord chord;
    VoicingOptions voicingOptions;
    bool isLocked{false};
    float durationBeats{4.0f}; // 4 beats = 1 bar in 4/4
    std::string customLabel{""};

    [[nodiscard]] std::string getDisplayLabel() const;
};

enum class RandomizerStyle : uint8_t
{
    Pop = 0,       // I-V-vi-IV, I-IV-vi-V, etc.
    Jazz,          // ii7-V7-Imaj7, I-vi-ii-V, etc.
    MinorSad,      // i-VI-III-VII, i-iv-v-i, etc.
    RnBSoul,       // Imaj7-vi7-ii7-V7, IVmaj7-iii7-vi7-ii7
    EDMModern,     // vi-IV-I-V, IV-V-vi-I
    AmbientChill   // Imaj9-IVmaj9-vi9-V
};

class Progression
{
public:
    Progression(Scale scale = Scale(PitchClass::C, ScaleType::Major));

    // Slots management
    [[nodiscard]] size_t getSlotCount() const noexcept { return mSlots.size(); }
    [[nodiscard]] const std::vector<ProgressionSlot>& getSlots() const noexcept { return mSlots; }
    [[nodiscard]] ProgressionSlot* getSlot(size_t index);
    [[nodiscard]] const ProgressionSlot* getSlot(size_t index) const;

    void addSlot(const ProgressionSlot& slot);
    void insertSlot(size_t index, const ProgressionSlot& slot);
    void removeSlot(size_t index);
    void clearSlots();
    void setSlotLocked(size_t index, bool locked);

    // Progression properties
    [[nodiscard]] const Scale& getScale() const noexcept { return mScale; }
    void setScale(const Scale& scale, bool updateUnlockedDiatonicChords = true);

    [[nodiscard]] float getBpm() const noexcept { return mBpm; }
    void setBpm(float bpm) noexcept { mBpm = std::clamp(bpm, 20.0f, 300.0f); }

    [[nodiscard]] bool isVoiceLeadingEnabled() const noexcept { return mEnableVoiceLeading; }
    void setVoiceLeadingEnabled(bool enable) noexcept { mEnableVoiceLeading = enable; }

    // Generate voiced notes for all slots
    [[nodiscard]] std::vector<std::vector<Note>> generateAllVoicings() const;

    // Musical Randomization
    void randomize(RandomizerStyle style = RandomizerStyle::Pop, int length = 4);

    // Presets
    static Progression createCommonProgression(const Scale& scale, RandomizerStyle style);

private:
    Scale mScale;
    std::vector<ProgressionSlot> mSlots;
    float mBpm{120.0f};
    bool mEnableVoiceLeading{true};
};

} // namespace ChordForge::Music
