#pragma once

#include "Note.h"
#include "Scale.h"
#include <vector>
#include <string>
#include <string_view>

namespace ChordForge::Music
{

enum class ChordQuality : uint8_t
{
    // Triads
    Major = 0,
    Minor,
    Diminished,
    Augmented,
    Sus2,
    Sus4,

    // 7ths
    Major7,
    Minor7,
    Dominant7,
    Diminished7,
    HalfDiminished7,
    MinorMajor7,

    // Extensions
    Add9,
    Major9,
    Minor9,
    Dominant9,
    Major11,
    Minor11,
    Dominant11,
    Major13,
    Minor13,
    Dominant13
};

class Chord
{
public:
    Chord();
    Chord(PitchClass root, ChordQuality quality, int octave = 4);
    Chord(PitchClass root, std::vector<int> semitoneIntervals, std::string qualityName, int octave = 4);

    // Diatonic chord generation by degree (1..7) from a scale
    static Chord fromScaleDegree(const Scale& scale, int degree, bool includeSeventh = false, int octave = 4);

    // Queries
    [[nodiscard]] PitchClass getRoot() const noexcept { return mRoot; }
    [[nodiscard]] ChordQuality getQuality() const noexcept { return mQuality; }
    [[nodiscard]] int getOctave() const noexcept { return mOctave; }
    [[nodiscard]] const std::vector<int>& getIntervals() const noexcept { return mIntervals; }
    [[nodiscard]] size_t getNoteCount() const noexcept { return mIntervals.size(); }

    // Chord names & Roman numerals
    [[nodiscard]] std::string getName(bool useSharps = true) const;
    [[nodiscard]] std::string getQualityName() const;
    [[nodiscard]] std::string getRomanNumeral() const { return mRomanNumeral; }
    void setRomanNumeral(std::string_view roman) { mRomanNumeral = roman; }

    // Notes in root position for given octave
    [[nodiscard]] std::vector<Note> getRootPositionNotes() const;

    // Pitch classes in chord
    [[nodiscard]] std::vector<PitchClass> getPitchClasses() const;

    // Factory methods & quality lookup
    static const std::vector<int>& getIntervalsForQuality(ChordQuality quality);
    static std::string_view getQualityName(ChordQuality quality);
    static ChordQuality identifyQuality(const std::vector<int>& intervals);

    // Roman numeral solver for diatonic degrees
    static std::string getRomanNumeralForDegree(int degree, ChordQuality quality);

private:
    PitchClass mRoot{PitchClass::C};
    ChordQuality mQuality{ChordQuality::Major};
    int mOctave{4};
    std::vector<int> mIntervals{0, 4, 7};
    std::string mRomanNumeral{"I"};
};

} // namespace ChordForge::Music
