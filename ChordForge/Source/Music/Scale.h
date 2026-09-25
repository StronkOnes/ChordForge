#pragma once

#include "Note.h"
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

namespace ChordForge::Music
{

enum class ScaleType : uint8_t
{
    Major = 0,         // Ionian: 0 2 4 5 7 9 11
    NaturalMinor,      // Aeolian: 0 2 3 5 7 8 10
    Dorian,            // 0 2 3 5 7 9 10
    Phrygian,          // 0 1 3 5 7 8 10
    Lydian,            // 0 2 4 6 7 9 11
    Mixolydian,        // 0 2 4 5 7 9 10
    Locrian,           // 0 1 3 5 6 8 10
    HarmonicMinor,     // 0 2 3 5 7 8 11
    MelodicMinor       // 0 2 3 5 7 9 11
};

class Scale
{
public:
    Scale(PitchClass root = PitchClass::C, ScaleType type = ScaleType::Major);

    [[nodiscard]] PitchClass getRoot() const noexcept { return mRoot; }
    [[nodiscard]] ScaleType getType() const noexcept { return mType; }
    [[nodiscard]] const std::vector<int>& getIntervals() const noexcept { return mIntervals; }

    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::string getFullName() const;

    // Scale notes across a given octave
    [[nodiscard]] std::vector<Note> getNotesInOctave(int octave) const;

    // Pitch classes included in this scale
    [[nodiscard]] std::vector<PitchClass> getPitchClasses() const;

    // Get note at a scale degree (1-indexed: 1 = root, 2 = 2nd degree, etc.)
    [[nodiscard]] Note getNoteAtDegree(int degree, int baseOctave = 4) const;

    // Scale mask (bitmask of 12 bits where bit i is set if pitch class i is in scale)
    [[nodiscard]] uint16_t getPitchClassMask() const noexcept;
    [[nodiscard]] bool contains(PitchClass pc) const noexcept;
    [[nodiscard]] bool contains(const Note& note) const noexcept;

    // Scale degree finder (returns 1..7 or 0 if not diatonic)
    [[nodiscard]] int getDegreeOfPitchClass(PitchClass pc) const;

    // Factory methods
    static const std::vector<int>& getIntervalsForType(ScaleType type);
    static std::string_view getScaleTypeName(ScaleType type);
    static std::vector<ScaleType> getAllScaleTypes();

private:
    PitchClass mRoot;
    ScaleType mType;
    std::vector<int> mIntervals;
    uint16_t mPitchClassMask{0};

    void computeMask();
};

} // namespace ChordForge::Music
