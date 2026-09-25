#pragma once

#include "Chord.h"
#include "Note.h"
#include <vector>
#include <string_view>
#include <cstdint>

namespace ChordForge::Music
{

enum class VoicingStyle : uint8_t
{
    Close = 0,
    Open,
    Wide,
    Drop2,
    Piano,
    Guitar,
    Pad,
    BassPlusChord
};

struct VoicingOptions
{
    VoicingStyle style{VoicingStyle::Close};
    int inversion{0};       // 0 = root, 1 = 1st, 2 = 2nd, 3 = 3rd
    int octaveShift{0};     // -2 .. +2
    bool humanizeVelocity{false};
    uint8_t baseVelocity{96};
};

class Voicing
{
public:
    // Generate voiced MIDI notes for a given chord and configuration
    static std::vector<Note> generate(const Chord& chord, const VoicingOptions& options);

    // Apply inversion to note array (in-place or returning new array)
    static std::vector<Note> applyInversion(const std::vector<Note>& notes, int inversion);

    // Apply specific styles
    static std::vector<Note> makeClose(const std::vector<Note>& notes);
    static std::vector<Note> makeOpen(const std::vector<Note>& notes);
    static std::vector<Note> makeWide(const std::vector<Note>& notes);
    static std::vector<Note> makeDrop2(const std::vector<Note>& notes);
    static std::vector<Note> makePiano(const std::vector<Note>& notes, PitchClass root);
    static std::vector<Note> makeGuitar(const std::vector<Note>& notes, PitchClass root);
    static std::vector<Note> makePad(const std::vector<Note>& notes);
    static std::vector<Note> makeBassPlusChord(const std::vector<Note>& notes, PitchClass root);

    // Style name helpers
    static std::string_view getStyleName(VoicingStyle style);
    static std::vector<VoicingStyle> getAllStyles();
};

} // namespace ChordForge::Music
