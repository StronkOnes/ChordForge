#include "Note.h"
#include <algorithm>
#include <cmath>
#include <array>

namespace ChordForge::Music
{

namespace
{
    constexpr std::array<const char*, 12> SHARP_NAMES = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    constexpr std::array<const char*, 12> FLAT_NAMES = {
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
    };
}

Note::Note(int midiNoteNumber)
    : mMidiNumber(clampMidi(midiNoteNumber))
{
}

Note::Note(PitchClass pitchClass, int octave)
{
    const int pc = static_cast<int>(pitchClass);
    // MIDI note 0 is C-1, C0 is 12, C4 is 60 -> (octave + 1) * 12 + pc
    const int calculated = (octave + 1) * 12 + (pc % 12);
    mMidiNumber = clampMidi(calculated);
}

PitchClass Note::getPitchClass() const noexcept
{
    return static_cast<PitchClass>((mMidiNumber % 12 + 12) % 12);
}

int Note::getPitchClassNumber() const noexcept
{
    return (mMidiNumber % 12 + 12) % 12;
}

int Note::getOctave() const noexcept
{
    // MIDI 60 (C4) -> 60/12 - 1 = 4
    return (mMidiNumber / 12) - 1;
}

double Note::getFrequencyHz(double a4TuningHz) const noexcept
{
    // A4 = MIDI note 69
    return a4TuningHz * std::pow(2.0, (mMidiNumber - 69) / 12.0);
}

std::string Note::getName(bool useSharps) const
{
    const int pc = getPitchClassNumber();
    return useSharps ? SHARP_NAMES[pc] : FLAT_NAMES[pc];
}

std::string Note::getFullName(bool useSharps) const
{
    return getName(useSharps) + std::to_string(getOctave());
}

Note Note::transposedBy(int semitones) const
{
    return Note(clampMidi(mMidiNumber + semitones));
}

Note Note::withOctave(int newOctave) const
{
    return Note(getPitchClass(), newOctave);
}

int Note::clampMidi(int noteNumber) noexcept
{
    return std::clamp(noteNumber, 0, 127);
}

PitchClass Note::parsePitchClass(std::string_view name)
{
    if (name.empty()) return PitchClass::Unknown;

    char base = static_cast<char>(std::toupper(name[0]));
    int val = 0;
    switch (base)
    {
        case 'C': val = 0; break;
        case 'D': val = 2; break;
        case 'E': val = 4; break;
        case 'F': val = 5; break;
        case 'G': val = 7; break;
        case 'A': val = 9; break;
        case 'B': val = 11; break;
        default: return PitchClass::Unknown;
    }

    if (name.size() > 1)
    {
        if (name[1] == '#' || name[1] == 's' || name[1] == 'S')
            val = (val + 1) % 12;
        else if (name[1] == 'b' || name[1] == 'B')
            val = (val + 11) % 12;
    }

    return static_cast<PitchClass>(val);
}

std::string_view Note::pitchClassName(PitchClass pc, bool useSharps)
{
    const int idx = static_cast<int>(pc);
    if (idx < 0 || idx >= 12) return "Unknown";
    return useSharps ? SHARP_NAMES[idx] : FLAT_NAMES[idx];
}

} // namespace ChordForge::Music
