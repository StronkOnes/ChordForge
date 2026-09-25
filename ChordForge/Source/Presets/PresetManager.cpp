#include "PresetManager.h"
#include <algorithm>

namespace ChordForge::Presets
{

juce::ValueTree Preset::toValueTree() const
{
    juce::ValueTree vt("Preset");
    vt.setProperty("name", juce::String::fromUTF8(name.c_str()), nullptr);
    vt.setProperty("category", juce::String::fromUTF8(category.c_str()), nullptr);
    vt.setProperty("key", static_cast<int>(key), nullptr);
    vt.setProperty("scaleType", static_cast<int>(scaleType), nullptr);
    vt.setProperty("voicing", static_cast<int>(defaultVoicing), nullptr);
    vt.setProperty("inversion", defaultInversion, nullptr);
    vt.setProperty("octaveShift", defaultOctaveShift, nullptr);
    vt.setProperty("bpm", bpm, nullptr);

    juce::ValueTree slotsTree("Slots");
    const auto& slots = progression.getSlots();
    for (size_t i = 0; i < slots.size(); ++i)
    {
        juce::ValueTree s("Slot");
        s.setProperty("root", static_cast<int>(slots[i].chord.getRoot()), nullptr);
        s.setProperty("quality", static_cast<int>(slots[i].chord.getQuality()), nullptr);
        s.setProperty("roman", juce::String::fromUTF8(slots[i].chord.getRomanNumeral().c_str()), nullptr);
        s.setProperty("duration", slots[i].durationBeats, nullptr);
        s.setProperty("locked", slots[i].isLocked, nullptr);
        slotsTree.addChild(s, -1, nullptr);
    }
    vt.addChild(slotsTree, -1, nullptr);

    return vt;
}

Preset Preset::fromValueTree(const juce::ValueTree& vt)
{
    Preset p;
    p.name = vt.getProperty("name").toString().toStdString();
    p.category = vt.getProperty("category").toString().toStdString();
    p.key = static_cast<Music::PitchClass>(static_cast<int>(vt.getProperty("key", 0)));
    p.scaleType = static_cast<Music::ScaleType>(static_cast<int>(vt.getProperty("scaleType", 0)));
    p.defaultVoicing = static_cast<Music::VoicingStyle>(static_cast<int>(vt.getProperty("voicing", 0)));
    p.defaultInversion = vt.getProperty("inversion", 0);
    p.defaultOctaveShift = vt.getProperty("octaveShift", 0);
    p.bpm = vt.getProperty("bpm", 120.0f);

    Music::Scale sc(p.key, p.scaleType);
    p.progression = Music::Progression(sc);
    p.progression.clearSlots();

    juce::ValueTree slotsTree = vt.getChildWithName("Slots");
    for (int i = 0; i < slotsTree.getNumChildren(); ++i)
    {
        juce::ValueTree s = slotsTree.getChild(i);
        Music::ProgressionSlot slot;
        auto root = static_cast<Music::PitchClass>(static_cast<int>(s.getProperty("root", 0)));
        auto quality = static_cast<Music::ChordQuality>(static_cast<int>(s.getProperty("quality", 0)));
        slot.chord = Music::Chord(root, quality);
        slot.chord.setRomanNumeral(s.getProperty("roman").toString().toStdString());
        slot.durationBeats = s.getProperty("duration", 4.0f);
        slot.isLocked = s.getProperty("locked", false);
        p.progression.addSlot(slot);
    }

    return p;
}

PresetManager::PresetManager()
{
    initializeFactoryPresets();
}

const Preset* PresetManager::getPreset(size_t index) const
{
    if (index >= mPresets.size()) return nullptr;
    return &mPresets[index];
}

const Preset* PresetManager::findPreset(const std::string& name) const
{
    for (const auto& p : mPresets)
    {
        if (p.name == name) return &p;
    }
    return nullptr;
}

std::vector<std::string> PresetManager::getCategories() const
{
    std::vector<std::string> cats;
    for (const auto& p : mPresets)
    {
        if (std::find(cats.begin(), cats.end(), p.category) == cats.end())
        {
            cats.push_back(p.category);
        }
    }
    return cats;
}

std::vector<Preset> PresetManager::getPresetsForCategory(const std::string& category) const
{
    std::vector<Preset> result;
    for (const auto& p : mPresets)
    {
        if (p.category == category)
        {
            result.push_back(p);
        }
    }
    return result;
}

void PresetManager::initializeFactoryPresets()
{
    mPresets.clear();

    // Helper lambda to create preset
    auto addFactory = [&](
        const std::string& name,
        const std::string& category,
        Music::PitchClass key,
        Music::ScaleType scaleType,
        const std::vector<int>& degrees,
        bool use7ths,
        Music::VoicingStyle style,
        float bpm
    ) {
        Preset p;
        p.name = name;
        p.category = category;
        p.key = key;
        p.scaleType = scaleType;
        p.defaultVoicing = style;
        p.bpm = bpm;

        Music::Scale scale(key, scaleType);
        p.progression = Music::Progression(scale);
        p.progression.clearSlots();

        for (int deg : degrees)
        {
            Music::ProgressionSlot slot;
            slot.chord = Music::Chord::fromScaleDegree(scale, deg, use7ths, 4);
            slot.voicingOptions.style = style;
            slot.durationBeats = 4.0f;
            p.progression.addSlot(slot);
        }

        mPresets.push_back(p);
    };

    // Pop
    addFactory("Four Chords of Pop", "Pop", Music::PitchClass::C, Music::ScaleType::Major, {1, 5, 6, 4}, false, Music::VoicingStyle::Piano, 120.0f);
    addFactory("Anthem Drive", "Pop", Music::PitchClass::G, Music::ScaleType::Major, {6, 4, 1, 5}, false, Music::VoicingStyle::Wide, 124.0f);

    // Hip Hop / Trap
    addFactory("Dark Trap Loop", "Hip Hop", Music::PitchClass::Cs, Music::ScaleType::NaturalMinor, {1, 6, 3, 7}, false, Music::VoicingStyle::Close, 140.0f);
    addFactory("Chill Boom Bap", "Hip Hop", Music::PitchClass::D, Music::ScaleType::Dorian, {1, 4, 2, 5}, true, Music::VoicingStyle::Drop2, 90.0f);

    // R&B / Soul
    addFactory("Neo Soul Groove", "R&B", Music::PitchClass::F, Music::ScaleType::Major, {4, 3, 6, 2}, true, Music::VoicingStyle::Drop2, 85.0f);
    addFactory("Midnight Velvet", "Soul", Music::PitchClass::As, Music::ScaleType::NaturalMinor, {1, 4, 6, 5}, true, Music::VoicingStyle::Piano, 78.0f);

    // Jazz
    addFactory("Standard 2-5-1", "Jazz", Music::PitchClass::C, Music::ScaleType::Major, {2, 5, 1, 6}, true, Music::VoicingStyle::Drop2, 110.0f);
    addFactory("Autumn Cadence", "Jazz", Music::PitchClass::G, Music::ScaleType::Major, {1, 6, 2, 5}, true, Music::VoicingStyle::Piano, 130.0f);

    // Lo-Fi
    addFactory("Sunday Coffee", "Lo-Fi", Music::PitchClass::E, Music::ScaleType::Major, {4, 1, 2, 5}, true, Music::VoicingStyle::Close, 75.0f);
    addFactory("Rainy Window", "Lo-Fi", Music::PitchClass::A, Music::ScaleType::NaturalMinor, {1, 7, 6, 5}, true, Music::VoicingStyle::Drop2, 82.0f);

    // Cinematic & Ambient
    addFactory("Ethereal Dawn", "Ambient", Music::PitchClass::D, Music::ScaleType::Lydian, {1, 2, 5, 1}, false, Music::VoicingStyle::Pad, 65.0f);
    addFactory("Hero's Journey", "Cinematic", Music::PitchClass::D, Music::ScaleType::NaturalMinor, {1, 6, 3, 7}, false, Music::VoicingStyle::Wide, 100.0f);

    // House / EDM
    addFactory("Classic Club Chords", "House", Music::PitchClass::A, Music::ScaleType::NaturalMinor, {1, 6, 4, 7}, false, Music::VoicingStyle::Piano, 126.0f);
    addFactory("Festival Uplift", "House", Music::PitchClass::F, Music::ScaleType::Major, {6, 4, 1, 5}, false, Music::VoicingStyle::Wide, 128.0f);
}

} // namespace ChordForge::Presets
