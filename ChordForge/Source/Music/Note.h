#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace ChordForge::Music
{

enum class PitchClass : uint8_t
{
    C = 0,
    Cs = 1,
    D = 2,
    Ds = 3,
    E = 4,
    F = 5,
    Fs = 6,
    G = 7,
    Gs = 8,
    A = 9,
    As = 10,
    B = 11,
    Unknown = 255
};

class Note
{
public:
    explicit Note(int midiNoteNumber = 60); // Default C4 = 60
    Note(PitchClass pitchClass, int octave);

    // Queries
    [[nodiscard]] int getMidiNumber() const noexcept { return mMidiNumber; }
    [[nodiscard]] PitchClass getPitchClass() const noexcept;
    [[nodiscard]] int getPitchClassNumber() const noexcept; // 0..11
    [[nodiscard]] int getOctave() const noexcept;
    [[nodiscard]] double getFrequencyHz(double a4TuningHz = 440.0) const noexcept;

    // Names and display
    [[nodiscard]] std::string getName(bool useSharps = true) const;
    [[nodiscard]] std::string getFullName(bool useSharps = true) const; // e.g. "C4", "F#3"

    // Manipulations
    [[nodiscard]] Note transposedBy(int semitones) const;
    [[nodiscard]] Note withOctave(int newOctave) const;

    // Comparisons
    bool operator==(const Note& other) const noexcept { return mMidiNumber == other.mMidiNumber; }
    bool operator!=(const Note& other) const noexcept { return mMidiNumber != other.mMidiNumber; }
    bool operator<(const Note& other) const noexcept { return mMidiNumber < other.mMidiNumber; }
    bool operator>(const Note& other) const noexcept { return mMidiNumber > other.mMidiNumber; }

    // Helpers
    static PitchClass parsePitchClass(std::string_view name);
    static std::string_view pitchClassName(PitchClass pc, bool useSharps = true);
    static bool isValidMidi(int noteNumber) noexcept { return noteNumber >= 0 && noteNumber <= 127; }
    static int clampMidi(int noteNumber) noexcept;

private:
    int mMidiNumber{60};
};

} // namespace ChordForge::Music
