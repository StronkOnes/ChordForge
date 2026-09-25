#include "Progression.h"
#include "VoiceLeading.h"
#include <random>
#include <array>

namespace ChordForge::Music
{

std::string ProgressionSlot::getDisplayLabel() const
{
    if (!customLabel.empty()) return customLabel;
    if (!chord.getRomanNumeral().empty())
    {
        return chord.getRomanNumeral() + " (" + chord.getName() + ")";
    }
    return chord.getName();
}

Progression::Progression(Scale scale)
    : mScale(scale)
{
    // Initialize with a classic 4-chord progression: I - V - vi - IV
    const bool isMin = (scale.getType() == ScaleType::NaturalMinor || scale.getType() == ScaleType::HarmonicMinor);
    const std::vector<int> degrees = isMin ? std::vector<int>{1, 6, 3, 7} : std::vector<int>{1, 5, 6, 4};

    for (int deg : degrees)
    {
        ProgressionSlot slot;
        slot.chord = Chord::fromScaleDegree(mScale, deg, false, 4);
        slot.voicingOptions.style = VoicingStyle::Close;
        slot.voicingOptions.inversion = 0;
        slot.voicingOptions.octaveShift = 0;
        slot.durationBeats = 4.0f;
        mSlots.push_back(slot);
    }
}

ProgressionSlot* Progression::getSlot(size_t index)
{
    if (index >= mSlots.size()) return nullptr;
    return &mSlots[index];
}

const ProgressionSlot* Progression::getSlot(size_t index) const
{
    if (index >= mSlots.size()) return nullptr;
    return &mSlots[index];
}

void Progression::addSlot(const ProgressionSlot& slot)
{
    mSlots.push_back(slot);
}

void Progression::insertSlot(size_t index, const ProgressionSlot& slot)
{
    if (index >= mSlots.size())
    {
        mSlots.push_back(slot);
    }
    else
    {
        mSlots.insert(mSlots.begin() + index, slot);
    }
}

void Progression::removeSlot(size_t index)
{
    if (index < mSlots.size())
    {
        mSlots.erase(mSlots.begin() + index);
    }
}

void Progression::clearSlots()
{
    mSlots.clear();
}

void Progression::setSlotLocked(size_t index, bool locked)
{
    if (index < mSlots.size())
    {
        mSlots[index].isLocked = locked;
    }
}

void Progression::setScale(const Scale& scale, bool updateUnlockedDiatonicChords)
{
    mScale = scale;
    if (!updateUnlockedDiatonicChords) return;

    // Update chords in unlocked slots using the new scale
    for (auto& slot : mSlots)
    {
        if (slot.isLocked) continue;
        int deg = mScale.getDegreeOfPitchClass(slot.chord.getRoot());
        if (deg == 0) deg = 1;
        const bool has7th = (slot.chord.getIntervals().size() >= 4);
        slot.chord = Chord::fromScaleDegree(mScale, deg, has7th, slot.chord.getOctave());
    }
}

std::vector<std::vector<Note>> Progression::generateAllVoicings() const
{
    if (mSlots.empty()) return {};

    std::vector<std::vector<Note>> result;
    result.reserve(mSlots.size());

    if (!mEnableVoiceLeading)
    {
        for (const auto& slot : mSlots)
        {
            result.push_back(Voicing::generate(slot.chord, slot.voicingOptions));
        }
    }
    else
    {
        // Voice leading applied sequentially
        for (size_t i = 0; i < mSlots.size(); ++i)
        {
            if (i == 0)
            {
                result.push_back(Voicing::generate(mSlots[0].chord, mSlots[0].voicingOptions));
            }
            else
            {
                auto optimal = VoiceLeading::findOptimalVoicing(
                    result.back(),
                    mSlots[i].chord,
                    mSlots[i].voicingOptions.style
                );
                result.push_back(std::move(optimal));
            }
        }
    }

    return result;
}

void Progression::randomize(RandomizerStyle style, int length)
{
    // Musical heuristics - curated progression sequences
    struct PatternDef
    {
        std::vector<int> degrees;
        bool use7ths;
    };

    static const std::vector<PatternDef> POP_PATTERNS = {
        {{1, 5, 6, 4}, false},
        {{6, 4, 1, 5}, false},
        {{1, 4, 6, 5}, false},
        {{1, 6, 4, 5}, false},
        {{1, 4, 5, 4}, false}
    };

    static const std::vector<PatternDef> JAZZ_PATTERNS = {
        {{2, 5, 1, 6}, true},
        {{1, 6, 2, 5}, true},
        {{3, 6, 2, 5}, true},
        {{2, 5, 1, 1}, true}
    };

    static const std::vector<PatternDef> MINOR_PATTERNS = {
        {{1, 6, 3, 7}, false},
        {{1, 4, 5, 1}, false},
        {{1, 6, 7, 1}, false},
        {{1, 4, 6, 5}, false}
    };

    static const std::vector<PatternDef> RNB_PATTERNS = {
        {{4, 3, 6, 2}, true},
        {{1, 6, 2, 5}, true},
        {{4, 5, 3, 6}, true},
        {{2, 3, 4, 5}, true}
    };

    const std::vector<PatternDef>* selectedPatterns = &POP_PATTERNS;
    switch (style)
    {
        case RandomizerStyle::Pop:          selectedPatterns = &POP_PATTERNS; break;
        case RandomizerStyle::Jazz:         selectedPatterns = &JAZZ_PATTERNS; break;
        case RandomizerStyle::MinorSad:      selectedPatterns = &MINOR_PATTERNS; break;
        case RandomizerStyle::RnBSoul:       selectedPatterns = &RNB_PATTERNS; break;
        case RandomizerStyle::EDMModern:     selectedPatterns = &POP_PATTERNS; break;
        case RandomizerStyle::AmbientChill:   selectedPatterns = &RNB_PATTERNS; break;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, selectedPatterns->size() - 1);
    const PatternDef& chosen = (*selectedPatterns)[dist(gen)];

    // Adjust slots vector length if needed
    if (mSlots.size() < static_cast<size_t>(length))
    {
        while (mSlots.size() < static_cast<size_t>(length))
        {
            ProgressionSlot newSlot;
            newSlot.durationBeats = 4.0f;
            mSlots.push_back(newSlot);
        }
    }

    // Assign diatonic chords to unlocked slots
    for (size_t i = 0; i < static_cast<size_t>(length) && i < mSlots.size(); ++i)
    {
        if (mSlots[i].isLocked) continue; // Respect locked slots!

        int degree = chosen.degrees[i % chosen.degrees.size()];
        mSlots[i].chord = Chord::fromScaleDegree(mScale, degree, chosen.use7ths, 4);
    }
}

Progression Progression::createCommonProgression(const Scale& scale, RandomizerStyle style)
{
    Progression p(scale);
    p.randomize(style, 4);
    return p;
}

} // namespace ChordForge::Music
