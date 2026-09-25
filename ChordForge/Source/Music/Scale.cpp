#include "Scale.h"
#include <algorithm>

namespace ChordForge::Music
{

namespace
{
    const std::vector<int> MAJOR_INTERVALS         = {0, 2, 4, 5, 7, 9, 11};
    const std::vector<int> NATURAL_MINOR_INTERVALS = {0, 2, 3, 5, 7, 8, 10};
    const std::vector<int> DORIAN_INTERVALS        = {0, 2, 3, 5, 7, 9, 10};
    const std::vector<int> PHRYGIAN_INTERVALS      = {0, 1, 3, 5, 7, 8, 10};
    const std::vector<int> LYDIAN_INTERVALS        = {0, 2, 4, 6, 7, 9, 11};
    const std::vector<int> MIXOLYDIAN_INTERVALS    = {0, 2, 4, 5, 7, 9, 10};
    const std::vector<int> LOCRIAN_INTERVALS       = {0, 1, 3, 5, 6, 8, 10};
    const std::vector<int> HARMONIC_MINOR_INTERVALS= {0, 2, 3, 5, 7, 8, 11};
    const std::vector<int> MELODIC_MINOR_INTERVALS = {0, 2, 3, 5, 7, 9, 11};
}

Scale::Scale(PitchClass root, ScaleType type)
    : mRoot(root), mType(type), mIntervals(getIntervalsForType(type))
{
    computeMask();
}

void Scale::computeMask()
{
    mPitchClassMask = 0;
    const int rootVal = static_cast<int>(mRoot);
    for (int interval : mIntervals)
    {
        const int pc = (rootVal + interval) % 12;
        mPitchClassMask |= static_cast<uint16_t>(1 << pc);
    }
}

std::string Scale::getName() const
{
    return std::string(getScaleTypeName(mType));
}

std::string Scale::getFullName() const
{
    return std::string(Note::pitchClassName(mRoot, true)) + " " + getName();
}

std::vector<Note> Scale::getNotesInOctave(int octave) const
{
    std::vector<Note> notes;
    notes.reserve(mIntervals.size());
    const int rootMidi = Note(mRoot, octave).getMidiNumber();
    for (int interval : mIntervals)
    {
        notes.emplace_back(rootMidi + interval);
    }
    return notes;
}

std::vector<PitchClass> Scale::getPitchClasses() const
{
    std::vector<PitchClass> result;
    result.reserve(mIntervals.size());
    const int rootVal = static_cast<int>(mRoot);
    for (int interval : mIntervals)
    {
        result.push_back(static_cast<PitchClass>((rootVal + interval) % 12));
    }
    return result;
}

Note Scale::getNoteAtDegree(int degree, int baseOctave) const
{
    // degree is 1-based (1 = root, 2 = 2nd, etc.)
    if (mIntervals.empty()) return Note(mRoot, baseOctave);
    
    // Normalize degree: 1 -> index 0
    int index = degree - 1;
    int octaveShift = 0;
    const int numDegrees = static_cast<int>(mIntervals.size());

    while (index < 0)
    {
        index += numDegrees;
        octaveShift -= 1;
    }
    while (index >= numDegrees)
    {
        index -= numDegrees;
        octaveShift += 1;
    }

    const int rootMidi = Note(mRoot, baseOctave + octaveShift).getMidiNumber();
    return Note(rootMidi + mIntervals[index]);
}

uint16_t Scale::getPitchClassMask() const noexcept
{
    return mPitchClassMask;
}

bool Scale::contains(PitchClass pc) const noexcept
{
    const int bit = static_cast<int>(pc);
    return (mPitchClassMask & (1 << bit)) != 0;
}

bool Scale::contains(const Note& note) const noexcept
{
    return contains(note.getPitchClass());
}

int Scale::getDegreeOfPitchClass(PitchClass pc) const
{
    const int rootVal = static_cast<int>(mRoot);
    const int targetVal = static_cast<int>(pc);
    const int diff = (targetVal - rootVal + 12) % 12;

    for (size_t i = 0; i < mIntervals.size(); ++i)
    {
        if (mIntervals[i] == diff)
        {
            return static_cast<int>(i + 1); // 1-based
        }
    }
    return 0; // Not in scale
}

const std::vector<int>& Scale::getIntervalsForType(ScaleType type)
{
    switch (type)
    {
        case ScaleType::Major:         return MAJOR_INTERVALS;
        case ScaleType::NaturalMinor:  return NATURAL_MINOR_INTERVALS;
        case ScaleType::Dorian:        return DORIAN_INTERVALS;
        case ScaleType::Phrygian:      return PHRYGIAN_INTERVALS;
        case ScaleType::Lydian:        return LYDIAN_INTERVALS;
        case ScaleType::Mixolydian:    return MIXOLYDIAN_INTERVALS;
        case ScaleType::Locrian:       return LOCRIAN_INTERVALS;
        case ScaleType::HarmonicMinor: return HARMONIC_MINOR_INTERVALS;
        case ScaleType::MelodicMinor:  return MELODIC_MINOR_INTERVALS;
    }
    return MAJOR_INTERVALS;
}

std::string_view Scale::getScaleTypeName(ScaleType type)
{
    switch (type)
    {
        case ScaleType::Major:         return "Major";
        case ScaleType::NaturalMinor:  return "Natural Minor";
        case ScaleType::Dorian:        return "Dorian";
        case ScaleType::Phrygian:      return "Phrygian";
        case ScaleType::Lydian:        return "Lydian";
        case ScaleType::Mixolydian:    return "Mixolydian";
        case ScaleType::Locrian:       return "Locrian";
        case ScaleType::HarmonicMinor: return "Harmonic Minor";
        case ScaleType::MelodicMinor:  return "Melodic Minor";
    }
    return "Major";
}

std::vector<ScaleType> Scale::getAllScaleTypes()
{
    return {
        ScaleType::Major,
        ScaleType::NaturalMinor,
        ScaleType::Dorian,
        ScaleType::Phrygian,
        ScaleType::Lydian,
        ScaleType::Mixolydian,
        ScaleType::Locrian,
        ScaleType::HarmonicMinor,
        ScaleType::MelodicMinor
    };
}

} // namespace ChordForge::Music
