#pragma once

#include "../Music/Progression.h"
#include <juce_data_structures/juce_data_structures.h>
#include <string>
#include <vector>

namespace ChordForge::Presets
{

struct Preset
{
    std::string name;
    std::string category;
    Music::PitchClass key{Music::PitchClass::C};
    Music::ScaleType scaleType{Music::ScaleType::Major};
    Music::Progression progression;
    Music::VoicingStyle defaultVoicing{Music::VoicingStyle::Close};
    int defaultInversion{0};
    int defaultOctaveShift{0};
    float bpm{120.0f};

    [[nodiscard]] juce::ValueTree toValueTree() const;
    static Preset fromValueTree(const juce::ValueTree& vt);
};

class PresetManager
{
public:
    PresetManager();

    [[nodiscard]] const std::vector<Preset>& getPresets() const noexcept { return mPresets; }
    [[nodiscard]] const Preset* getPreset(size_t index) const;
    [[nodiscard]] const Preset* findPreset(const std::string& name) const;

    [[nodiscard]] std::vector<std::string> getCategories() const;
    [[nodiscard]] std::vector<Preset> getPresetsForCategory(const std::string& category) const;

private:
    std::vector<Preset> mPresets;

    void initializeFactoryPresets();
};

} // namespace ChordForge::Presets
