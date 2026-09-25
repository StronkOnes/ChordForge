#include "Chord.h"
#include <algorithm>

namespace ChordForge::Music
{

namespace
{
    // Interval templates
    const std::vector<int> INT_MAJOR           = {0, 4, 7};
    const std::vector<int> INT_MINOR           = {0, 3, 7};
    const std::vector<int> INT_DIMINISHED      = {0, 3, 6};
    const std::vector<int> INT_AUGMENTED       = {0, 4, 8};
    const std::vector<int> INT_SUS2            = {0, 2, 7};
    const std::vector<int> INT_SUS4            = {0, 5, 7};

    const std::vector<int> INT_MAJOR7          = {0, 4, 7, 11};
    const std::vector<int> INT_MINOR7          = {0, 3, 7, 10};
    const std::vector<int> INT_DOMINANT7       = {0, 4, 7, 10};
    const std::vector<int> INT_DIMINISHED7     = {0, 3, 6, 9};
    const std::vector<int> INT_HALF_DIM7       = {0, 3, 6, 10};
    const std::vector<int> INT_MINOR_MAJOR7    = {0, 3, 7, 11};

    const std::vector<int> INT_ADD9            = {0, 4, 7, 14};
    const std::vector<int> INT_MAJOR9          = {0, 4, 7, 11, 14};
    const std::vector<int> INT_MINOR9          = {0, 3, 7, 10, 14};
    const std::vector<int> INT_DOMINANT9       = {0, 4, 7, 10, 14};
    const std::vector<int> INT_MAJOR11         = {0, 4, 7, 11, 14, 17};
    const std::vector<int> INT_MINOR11         = {0, 3, 7, 10, 14, 17};
    const std::vector<int> INT_DOMINANT11      = {0, 4, 7, 10, 14, 17};
    const std::vector<int> INT_MAJOR13         = {0, 4, 7, 11, 14, 17, 21};
    const std::vector<int> INT_MINOR13         = {0, 3, 7, 10, 14, 17, 21};
    const std::vector<int> INT_DOMINANT13      = {0, 4, 7, 10, 14, 17, 21};
}

Chord::Chord()
    : mRoot(PitchClass::C), mQuality(ChordQuality::Major), mOctave(4),
      mIntervals(INT_MAJOR), mRomanNumeral("I")
{
}

Chord::Chord(PitchClass root, ChordQuality quality, int octave)
    : mRoot(root), mQuality(quality), mOctave(octave),
      mIntervals(getIntervalsForQuality(quality))
{
    mRomanNumeral = "";
}

Chord::Chord(PitchClass root, std::vector<int> semitoneIntervals, std::string qualityName, int octave)
    : mRoot(root), mOctave(octave), mIntervals(std::move(semitoneIntervals))
{
    mQuality = identifyQuality(mIntervals);
    mRomanNumeral = "";
}

Chord Chord::fromScaleDegree(const Scale& scale, int degree, bool includeSeventh, int octave)
{
    // Algorithmic diatonic chord generation:
    // Stacking thirds in the scale: degree, degree+2, degree+4 (and degree+6 for 7th)
    const int rootDeg = degree;
    const int thirdDeg = degree + 2;
    const int fifthDeg = degree + 4;
    const int seventhDeg = degree + 6;

    const Note rootNote = scale.getNoteAtDegree(rootDeg, octave);
    const Note thirdNote = scale.getNoteAtDegree(thirdDeg, octave);
    const Note fifthNote = scale.getNoteAtDegree(fifthDeg, octave);

    const int rootMidi = rootNote.getMidiNumber();
    const int thirdInterval = thirdNote.getMidiNumber() - rootMidi;
    const int fifthInterval = fifthNote.getMidiNumber() - rootMidi;

    std::vector<int> intervals = {0, thirdInterval, fifthInterval};
    if (includeSeventh)
    {
        const Note seventhNote = scale.getNoteAtDegree(seventhDeg, octave);
        intervals.push_back(seventhNote.getMidiNumber() - rootMidi);
    }

    const ChordQuality quality = identifyQuality(intervals);
    Chord chord(rootNote.getPitchClass(), intervals, std::string(getQualityName(quality)), octave);
    chord.mQuality = quality;
    chord.mRomanNumeral = getRomanNumeralForDegree(degree, quality);

    return chord;
}

std::string Chord::getName(bool useSharps) const
{
    std::string rootName = std::string(Note::pitchClassName(mRoot, useSharps));
    std::string qual = std::string(getQualityName(mQuality));
    return rootName + " " + qual;
}

std::string Chord::getQualityName() const
{
    return std::string(getQualityName(mQuality));
}

std::vector<Note> Chord::getRootPositionNotes() const
{
    std::vector<Note> notes;
    notes.reserve(mIntervals.size());
    const int rootMidi = Note(mRoot, mOctave).getMidiNumber();
    for (int interval : mIntervals)
    {
        notes.emplace_back(rootMidi + interval);
    }
    return notes;
}

std::vector<PitchClass> Chord::getPitchClasses() const
{
    std::vector<PitchClass> pcs;
    pcs.reserve(mIntervals.size());
    const int rootVal = static_cast<int>(mRoot);
    for (int interval : mIntervals)
    {
        pcs.push_back(static_cast<PitchClass>((rootVal + interval) % 12));
    }
    return pcs;
}

const std::vector<int>& Chord::getIntervalsForQuality(ChordQuality quality)
{
    switch (quality)
    {
        case ChordQuality::Major:           return INT_MAJOR;
        case ChordQuality::Minor:           return INT_MINOR;
        case ChordQuality::Diminished:      return INT_DIMINISHED;
        case ChordQuality::Augmented:       return INT_AUGMENTED;
        case ChordQuality::Sus2:            return INT_SUS2;
        case ChordQuality::Sus4:            return INT_SUS4;

        case ChordQuality::Major7:          return INT_MAJOR7;
        case ChordQuality::Minor7:          return INT_MINOR7;
        case ChordQuality::Dominant7:       return INT_DOMINANT7;
        case ChordQuality::Diminished7:     return INT_DIMINISHED7;
        case ChordQuality::HalfDiminished7: return INT_HALF_DIM7;
        case ChordQuality::MinorMajor7:     return INT_MINOR_MAJOR7;

        case ChordQuality::Add9:            return INT_ADD9;
        case ChordQuality::Major9:          return INT_MAJOR9;
        case ChordQuality::Minor9:          return INT_MINOR9;
        case ChordQuality::Dominant9:       return INT_DOMINANT9;
        case ChordQuality::Major11:         return INT_MAJOR11;
        case ChordQuality::Minor11:         return INT_MINOR11;
        case ChordQuality::Dominant11:      return INT_DOMINANT11;
        case ChordQuality::Major13:         return INT_MAJOR13;
        case ChordQuality::Minor13:         return INT_MINOR13;
        case ChordQuality::Dominant13:      return INT_DOMINANT13;
    }
    return INT_MAJOR;
}

std::string_view Chord::getQualityName(ChordQuality quality)
{
    switch (quality)
    {
        case ChordQuality::Major:           return "Major";
        case ChordQuality::Minor:           return "Minor";
        case ChordQuality::Diminished:      return "Diminished";
        case ChordQuality::Augmented:       return "Augmented";
        case ChordQuality::Sus2:            return "Sus2";
        case ChordQuality::Sus4:            return "Sus4";

        case ChordQuality::Major7:          return "Major 7";
        case ChordQuality::Minor7:          return "Minor 7";
        case ChordQuality::Dominant7:       return "Dominant 7";
        case ChordQuality::Diminished7:     return "Diminished 7";
        case ChordQuality::HalfDiminished7: return "Half-Diminished 7";
        case ChordQuality::MinorMajor7:     return "Minor-Major 7";

        case ChordQuality::Add9:            return "Add9";
        case ChordQuality::Major9:          return "Major 9";
        case ChordQuality::Minor9:          return "Minor 9";
        case ChordQuality::Dominant9:       return "Dominant 9";
        case ChordQuality::Major11:         return "Major 11";
        case ChordQuality::Minor11:         return "Minor 11";
        case ChordQuality::Dominant11:      return "Dominant 11";
        case ChordQuality::Major13:         return "Major 13";
        case ChordQuality::Minor13:         return "Minor 13";
        case ChordQuality::Dominant13:      return "Dominant 13";
    }
    return "Major";
}

ChordQuality Chord::identifyQuality(const std::vector<int>& intervals)
{
    if (intervals.size() == 3)
    {
        const int third = intervals[1] % 12;
        const int fifth = intervals[2] % 12;

        if (third == 4 && fifth == 7) return ChordQuality::Major;
        if (third == 3 && fifth == 7) return ChordQuality::Minor;
        if (third == 3 && fifth == 6) return ChordQuality::Diminished;
        if (third == 4 && fifth == 8) return ChordQuality::Augmented;
        if (third == 2 && fifth == 7) return ChordQuality::Sus2;
        if (third == 5 && fifth == 7) return ChordQuality::Sus4;
    }
    else if (intervals.size() == 4)
    {
        const int third = intervals[1] % 12;
        const int fifth = intervals[2] % 12;
        const int seventh = intervals[3] % 12;

        if (third == 4 && fifth == 7 && seventh == 11) return ChordQuality::Major7;
        if (third == 3 && fifth == 7 && seventh == 10) return ChordQuality::Minor7;
        if (third == 4 && fifth == 7 && seventh == 10) return ChordQuality::Dominant7;
        if (third == 3 && fifth == 6 && seventh == 9)  return ChordQuality::Diminished7;
        if (third == 3 && fifth == 6 && seventh == 10) return ChordQuality::HalfDiminished7;
        if (third == 3 && fifth == 7 && seventh == 11) return ChordQuality::MinorMajor7;
        if (third == 4 && fifth == 7 && seventh == 2)  return ChordQuality::Add9;
    }
    return ChordQuality::Major;
}

std::string Chord::getRomanNumeralForDegree(int degree, ChordQuality quality)
{
    static const char* const UPPER_ROMAN[] = {"I", "II", "III", "IV", "V", "VI", "VII"};
    static const char* const LOWER_ROMAN[] = {"i", "ii", "iii", "iv", "v", "vi", "vii"};

    const int idx = std::clamp(degree - 1, 0, 6);

    switch (quality)
    {
        case ChordQuality::Major:           return UPPER_ROMAN[idx];
        case ChordQuality::Minor:           return LOWER_ROMAN[idx];
        case ChordQuality::Diminished:      return std::string(LOWER_ROMAN[idx]) + "°";
        case ChordQuality::Augmented:       return std::string(UPPER_ROMAN[idx]) + "+";
        case ChordQuality::Sus2:            return std::string(UPPER_ROMAN[idx]) + "sus2";
        case ChordQuality::Sus4:            return std::string(UPPER_ROMAN[idx]) + "sus4";

        case ChordQuality::Major7:          return std::string(UPPER_ROMAN[idx]) + "maj7";
        case ChordQuality::Minor7:          return std::string(LOWER_ROMAN[idx]) + "7";
        case ChordQuality::Dominant7:       return std::string(UPPER_ROMAN[idx]) + "7";
        case ChordQuality::Diminished7:     return std::string(LOWER_ROMAN[idx]) + "°7";
        case ChordQuality::HalfDiminished7: return std::string(LOWER_ROMAN[idx]) + "ø7";
        case ChordQuality::MinorMajor7:     return std::string(LOWER_ROMAN[idx]) + "m(maj7)";

        case ChordQuality::Major9:          return std::string(UPPER_ROMAN[idx]) + "maj9";
        case ChordQuality::Minor9:          return std::string(LOWER_ROMAN[idx]) + "9";
        case ChordQuality::Dominant9:       return std::string(UPPER_ROMAN[idx]) + "9";
        default:                            return UPPER_ROMAN[idx];
    }
}

} // namespace ChordForge::Music
