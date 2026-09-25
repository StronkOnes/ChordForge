#include "../Music/Note.h"
#include "../Music/Scale.h"
#include "../Music/Chord.h"
#include "../Music/Voicing.h"
#include "../Music/VoiceLeading.h"
#include "../Music/Progression.h"
#include "../Assembly/OptimizedMath.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace ChordForge::Music;

static int gFailedTests = 0;
static int gPassedTests = 0;

#define EXPECT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            gFailedTests++; \
        } else { \
            gPassedTests++; \
        } \
    } while(0)

#define EXPECT_EQ(val1, val2, msg) \
    do { \
        if ((val1) != (val2)) { \
            std::cerr << "[FAIL] " << msg << " | Expected: " << (val2) << " Got: " << (val1) \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            gFailedTests++; \
        } else { \
            gPassedTests++; \
        } \
    } while(0)

void testNotesAndPitchClasses()
{
    std::cout << "Running testNotesAndPitchClasses...\n";
    Note c4(60);
    EXPECT_EQ(c4.getPitchClassNumber(), 0, "C4 pitch class number should be 0");
    EXPECT_EQ(c4.getOctave(), 4, "C4 octave should be 4");
    EXPECT_EQ(c4.getFullName(), "C4", "C4 full name should be C4");

    Note a4(69);
    EXPECT_EQ(a4.getFullName(), "A4", "A4 full name should be A4");
    EXPECT_TRUE(std::abs(a4.getFrequencyHz() - 440.0) < 0.01, "A4 frequency should be ~440Hz");

    Note fs3(PitchClass::Fs, 3);
    EXPECT_EQ(fs3.getFullName(), "F#3", "F#3 full name should match");
    EXPECT_EQ(fs3.getMidiNumber(), 54, "F#3 MIDI note should be 54");

    // Clamping checks
    Note outOfBoundsHigh(200);
    EXPECT_EQ(outOfBoundsHigh.getMidiNumber(), 127, "High note should clamp to 127");
    Note outOfBoundsLow(-10);
    EXPECT_EQ(outOfBoundsLow.getMidiNumber(), 0, "Low note should clamp to 0");
}

void testScales()
{
    std::cout << "Running testScales...\n";
    Scale cMajor(PitchClass::C, ScaleType::Major);
    const auto& intervals = cMajor.getIntervals();
    std::vector<int> expectedMajor = {0, 2, 4, 5, 7, 9, 11};
    EXPECT_EQ(intervals.size(), 7, "Major scale has 7 intervals");
    for (size_t i = 0; i < 7; ++i)
    {
        EXPECT_EQ(intervals[i], expectedMajor[i], "Major scale interval matches");
    }

    EXPECT_TRUE(cMajor.contains(PitchClass::C), "C Major contains C");
    EXPECT_TRUE(cMajor.contains(PitchClass::E), "C Major contains E");
    EXPECT_TRUE(cMajor.contains(PitchClass::B), "C Major contains B");
    EXPECT_TRUE(!cMajor.contains(PitchClass::Cs), "C Major does NOT contain C#");
    EXPECT_TRUE(!cMajor.contains(PitchClass::Fs), "C Major does NOT contain F#");

    Scale aMinor(PitchClass::A, ScaleType::NaturalMinor);
    std::vector<int> expectedMinor = {0, 2, 3, 5, 7, 8, 10};
    const auto& minorIntervals = aMinor.getIntervals();
    for (size_t i = 0; i < 7; ++i)
    {
        EXPECT_EQ(minorIntervals[i], expectedMinor[i], "Natural minor interval matches");
    }
}

void testCMajorDiatonicChords()
{
    std::cout << "Running testCMajorDiatonicChords (Section 30 specification)...\n";
    Scale cMajor(PitchClass::C, ScaleType::Major);

    // Degree 1: I = C Major (C E G) -> 60 64 67
    Chord I = Chord::fromScaleDegree(cMajor, 1, false, 4);
    EXPECT_EQ(I.getQuality(), ChordQuality::Major, "I is Major");
    EXPECT_EQ(I.getRoot(), PitchClass::C, "I root is C");
    auto nI = I.getRootPositionNotes();
    EXPECT_EQ(nI.size(), 3, "I has 3 notes");
    EXPECT_EQ(nI[0].getMidiNumber(), 60, "I root is 60 (C4)");
    EXPECT_EQ(nI[1].getMidiNumber(), 64, "I third is 64 (E4)");
    EXPECT_EQ(nI[2].getMidiNumber(), 67, "I fifth is 67 (G4)");

    // Degree 2: ii = D minor (D F A) -> 62 65 69
    Chord ii = Chord::fromScaleDegree(cMajor, 2, false, 4);
    EXPECT_EQ(ii.getQuality(), ChordQuality::Minor, "ii is Minor");
    EXPECT_EQ(ii.getRoot(), PitchClass::D, "ii root is D");
    auto nII = ii.getRootPositionNotes();
    EXPECT_EQ(nII[0].getMidiNumber(), 62, "ii root is 62 (D4)");
    EXPECT_EQ(nII[1].getMidiNumber(), 65, "ii third is 65 (F4)");
    EXPECT_EQ(nII[2].getMidiNumber(), 69, "ii fifth is 69 (A4)");

    // Degree 3: iii = E minor (E G B) -> 64 67 71
    Chord iii = Chord::fromScaleDegree(cMajor, 3, false, 4);
    EXPECT_EQ(iii.getQuality(), ChordQuality::Minor, "iii is Minor");
    auto nIII = iii.getRootPositionNotes();
    EXPECT_EQ(nIII[0].getMidiNumber(), 64, "iii root is E4");
    EXPECT_EQ(nIII[1].getMidiNumber(), 67, "iii third is G4");
    EXPECT_EQ(nIII[2].getMidiNumber(), 71, "iii fifth is B4");

    // Degree 4: IV = F major (F A C) -> 65 69 72
    Chord IV = Chord::fromScaleDegree(cMajor, 4, false, 4);
    EXPECT_EQ(IV.getQuality(), ChordQuality::Major, "IV is Major");
    auto nIV = IV.getRootPositionNotes();
    EXPECT_EQ(nIV[0].getMidiNumber(), 65, "IV root is F4");
    EXPECT_EQ(nIV[1].getMidiNumber(), 69, "IV third is A4");
    EXPECT_EQ(nIV[2].getMidiNumber(), 72, "IV fifth is C5");

    // Degree 5: V = G major (G B D) -> 67 71 74
    Chord V = Chord::fromScaleDegree(cMajor, 5, false, 4);
    EXPECT_EQ(V.getQuality(), ChordQuality::Major, "V is Major");
    auto nV = V.getRootPositionNotes();
    EXPECT_EQ(nV[0].getMidiNumber(), 67, "V root is G4");
    EXPECT_EQ(nV[1].getMidiNumber(), 71, "V third is B4");
    EXPECT_EQ(nV[2].getMidiNumber(), 74, "V fifth is D5");

    // Degree 6: vi = A minor (A C E) -> 69 72 76
    Chord vi = Chord::fromScaleDegree(cMajor, 6, false, 4);
    EXPECT_EQ(vi.getQuality(), ChordQuality::Minor, "vi is Minor");
    auto nVI = vi.getRootPositionNotes();
    EXPECT_EQ(nVI[0].getMidiNumber(), 69, "vi root is A4");
    EXPECT_EQ(nVI[1].getMidiNumber(), 72, "vi third is C5");
    EXPECT_EQ(nVI[2].getMidiNumber(), 76, "vi fifth is E5");

    // Degree 7: vii° = B diminished (B D F) -> 71 74 77
    Chord vii = Chord::fromScaleDegree(cMajor, 7, false, 4);
    EXPECT_EQ(vii.getQuality(), ChordQuality::Diminished, "vii° is Diminished");
    auto nVII = vii.getRootPositionNotes();
    EXPECT_EQ(nVII[0].getMidiNumber(), 71, "vii° root is B4");
    EXPECT_EQ(nVII[1].getMidiNumber(), 74, "vii° third is D5");
    EXPECT_EQ(nVII[2].getMidiNumber(), 77, "vii° fifth is F5");
}

void testInversions()
{
    std::cout << "Running testInversions (Section 30 specification)...\n";
    // C Major: Root = C4 E4 G4 (60, 64, 67)
    Chord cMaj(PitchClass::C, ChordQuality::Major, 4);

    VoicingOptions optsRoot;
    optsRoot.inversion = 0;
    auto rootNotes = Voicing::generate(cMaj, optsRoot);
    EXPECT_EQ(rootNotes[0].getMidiNumber(), 60, "Root: C4");
    EXPECT_EQ(rootNotes[1].getMidiNumber(), 64, "Root: E4");
    EXPECT_EQ(rootNotes[2].getMidiNumber(), 67, "Root: G4");

    // 1st Inversion: E4 G4 C5 (64, 67, 72)
    VoicingOptions opts1st;
    opts1st.inversion = 1;
    auto inv1Notes = Voicing::generate(cMaj, opts1st);
    EXPECT_EQ(inv1Notes[0].getMidiNumber(), 64, "1st Inv: E4");
    EXPECT_EQ(inv1Notes[1].getMidiNumber(), 67, "1st Inv: G4");
    EXPECT_EQ(inv1Notes[2].getMidiNumber(), 72, "1st Inv: C5");

    // 2nd Inversion: G4 C5 E5 (67, 72, 76)
    VoicingOptions opts2nd;
    opts2nd.inversion = 2;
    auto inv2Notes = Voicing::generate(cMaj, opts2nd);
    EXPECT_EQ(inv2Notes[0].getMidiNumber(), 67, "2nd Inv: G4");
    EXPECT_EQ(inv2Notes[1].getMidiNumber(), 72, "2nd Inv: C5");
    EXPECT_EQ(inv2Notes[2].getMidiNumber(), 76, "2nd Inv: E5");
}

void testSeventhChords()
{
    std::cout << "Running testSeventhChords...\n";
    Scale cMajor(PitchClass::C, ScaleType::Major);

    // Imaj7 = C E G B (60, 64, 67, 71)
    Chord Imaj7 = Chord::fromScaleDegree(cMajor, 1, true, 4);
    EXPECT_EQ(Imaj7.getQuality(), ChordQuality::Major7, "I with 7th is Major7");
    auto nI = Imaj7.getRootPositionNotes();
    EXPECT_EQ(nI[3].getMidiNumber(), 71, "Imaj7 seventh note is B4 (71)");

    // V7 = G B D F (67, 71, 74, 77)
    Chord V7 = Chord::fromScaleDegree(cMajor, 5, true, 4);
    EXPECT_EQ(V7.getQuality(), ChordQuality::Dominant7, "V with 7th is Dominant7");
    auto nV = V7.getRootPositionNotes();
    EXPECT_EQ(nV[3].getMidiNumber(), 77, "V7 seventh note is F5 (77)");

    // ii7 = D F A C (62, 65, 69, 72)
    Chord ii7 = Chord::fromScaleDegree(cMajor, 2, true, 4);
    EXPECT_EQ(ii7.getQuality(), ChordQuality::Minor7, "ii with 7th is Minor7");

    // viiø7 = B D F A (71, 74, 77, 81)
    Chord vii7 = Chord::fromScaleDegree(cMajor, 7, true, 4);
    EXPECT_EQ(vii7.getQuality(), ChordQuality::HalfDiminished7, "vii with 7th is HalfDiminished7");
}

void testVoiceLeading()
{
    std::cout << "Running testVoiceLeading...\n";
    Chord cMaj(PitchClass::C, ChordQuality::Major, 4); // C4 E4 G4 (60, 64, 67)
    Chord gMaj(PitchClass::G, ChordQuality::Major, 4); // G4 B4 D5 (67, 71, 74)

    VoicingOptions opts;
    opts.style = VoicingStyle::Close;
    auto voicedC = Voicing::generate(cMaj, opts);

    // Smooth voice leading to G Major should produce B3 D4 G4 (59, 62, 67) or similar smooth voicing
    // rather than jumping all voices upward by a 5th.
    auto optimalG = VoiceLeading::findOptimalVoicing(voicedC, gMaj, VoicingStyle::Close);
    float costOptimal = VoiceLeading::calculateCost(voicedC, optimalG);

    auto unguidedG = Voicing::generate(gMaj, opts);
    float costUnguided = VoiceLeading::calculateCost(voicedC, unguidedG);

    EXPECT_TRUE(costOptimal <= costUnguided, "Voice leading cost should be <= raw jump");
    EXPECT_TRUE(!optimalG.empty(), "Optimal voicing must not be empty");
}

void testOptimizedMath()
{
    std::cout << "Running testOptimizedMath...\n";
    int notes[4] = {60, 64, 67, 71};
    int output[4] = {0, 0, 0, 0};

    ChordForge::Optimized::batchTransposeNotes(notes, output, 4, 12);
    EXPECT_EQ(output[0], 72, "Transposed note 0 matches 72");
    EXPECT_EQ(output[1], 76, "Transposed note 1 matches 76");
    EXPECT_EQ(output[2], 79, "Transposed note 2 matches 79");
    EXPECT_EQ(output[3], 83, "Transposed note 3 matches 83");

    uint16_t mask = ChordForge::Optimized::computePitchClassMask(notes, 4);
    // C(0), E(4), G(7), B(11) -> bits 0, 4, 7, 11 set
    uint16_t expectedMask = (1 << 0) | (1 << 4) | (1 << 7) | (1 << 11);
    EXPECT_EQ(mask, expectedMask, "Pitch class mask matches");

    auto bench = ChordForge::Optimized::runTransposeBenchmark(1000);
    EXPECT_TRUE(bench.throughputOpsPerSec > 0.0, "Benchmark returns positive throughput");
}

int main()
{
    std::cout << "===========================================\n";
    std::cout << "CHORDFORGE MUSIC THEORY & MIDI TEST SUITE\n";
    std::cout << "===========================================\n";

    testNotesAndPitchClasses();
    testScales();
    testCMajorDiatonicChords();
    testInversions();
    testSeventhChords();
    testVoiceLeading();
    testOptimizedMath();

    std::cout << "\n-------------------------------------------\n";
    std::cout << "TOTAL PASSED: " << gPassedTests << "\n";
    std::cout << "TOTAL FAILED: " << gFailedTests << "\n";
    std::cout << "-------------------------------------------\n";

    return (gFailedTests == 0) ? 0 : 1;
}
