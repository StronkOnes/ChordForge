#include "Voicing.h"
#include <algorithm>
#include <set>

namespace ChordForge::Music
{

std::vector<Note> Voicing::generate(const Chord& chord, const VoicingOptions& options)
{
    // Start with root position notes at chord base octave
    std::vector<Note> baseNotes = chord.getRootPositionNotes();
    if (baseNotes.empty()) return {};

    // 1. Apply inversion
    std::vector<Note> inverted = applyInversion(baseNotes, options.inversion);

    // 2. Apply voicing style
    std::vector<Note> voiced;
    switch (options.style)
    {
        case VoicingStyle::Close:
            voiced = makeClose(inverted);
            break;
        case VoicingStyle::Open:
            voiced = makeOpen(inverted);
            break;
        case VoicingStyle::Wide:
            voiced = makeWide(inverted);
            break;
        case VoicingStyle::Drop2:
            voiced = makeDrop2(inverted);
            break;
        case VoicingStyle::Piano:
            voiced = makePiano(inverted, chord.getRoot());
            break;
        case VoicingStyle::Guitar:
            voiced = makeGuitar(inverted, chord.getRoot());
            break;
        case VoicingStyle::Pad:
            voiced = makePad(inverted);
            break;
        case VoicingStyle::BassPlusChord:
            voiced = makeBassPlusChord(inverted, chord.getRoot());
            break;
    }

    // 3. Apply global octave shift
    const int semitoneShift = options.octaveShift * 12;
    std::vector<Note> result;
    result.reserve(voiced.size());

    for (const auto& note : voiced)
    {
        const int shifted = note.getMidiNumber() + semitoneShift;
        result.emplace_back(Note::clampMidi(shifted));
    }

    // Sort ascending by pitch
    std::sort(result.begin(), result.end());

    // Deduplicate identical MIDI notes
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}

std::vector<Note> Voicing::applyInversion(const std::vector<Note>& notes, int inversion)
{
    if (notes.empty()) return {};
    std::vector<Note> result = notes;
    const int count = static_cast<int>(result.size());
    const int effectiveInversion = (inversion % count + count) % count;

    for (int i = 0; i < effectiveInversion; ++i)
    {
        Note lowest = result.front();
        result.erase(result.begin());
        result.push_back(lowest.transposedBy(12));
    }
    return result;
}

std::vector<Note> Voicing::makeClose(const std::vector<Note>& notes)
{
    return notes;
}

std::vector<Note> Voicing::makeOpen(const std::vector<Note>& notes)
{
    if (notes.size() < 3) return notes;
    std::vector<Note> result = notes;
    // Spread by lifting the 2nd note (index 1) up an octave, or dropping lowest
    result[1] = result[1].transposedBy(12);
    return result;
}

std::vector<Note> Voicing::makeWide(const std::vector<Note>& notes)
{
    if (notes.empty()) return notes;
    std::vector<Note> result = notes;
    // Spread voices alternating: bass dropped -12, upper voices raised +12
    if (!result.empty())
    {
        result[0] = result[0].transposedBy(-12);
    }
    if (result.size() >= 3)
    {
        result.back() = result.back().transposedBy(12);
    }
    return result;
}

std::vector<Note> Voicing::makeDrop2(const std::vector<Note>& notes)
{
    // Drop 2: In a 4-part (or 3-part) chord arranged in close position,
    // take the second voice from the top and drop it down an octave (12 semitones).
    if (notes.size() < 3) return notes;
    std::vector<Note> sorted = notes;
    std::sort(sorted.begin(), sorted.end());

    const size_t dropIndex = sorted.size() - 2; // 2nd voice from top
    sorted[dropIndex] = sorted[dropIndex].transposedBy(-12);

    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

std::vector<Note> Voicing::makePiano(const std::vector<Note>& notes, PitchClass root)
{
    // Piano style: Left hand plays bass root (and fifth) in Octave 2 or 3,
    // right hand plays the remaining chord tones in Octave 4/5
    std::vector<Note> result;
    const Note bass(root, 2);
    result.push_back(bass);

    for (const auto& n : notes)
    {
        // Place upper chord tones in octave 4-5
        int targetOctave = n.getOctave();
        if (targetOctave < 4) targetOctave += 1;
        result.push_back(n.withOctave(targetOctave));
    }
    return result;
}

std::vector<Note> Voicing::makeGuitar(const std::vector<Note>& notes, PitchClass root)
{
    // Guitar style: Root on low E or A string (octave 2 or 3), followed by 5th, root, 3rd, etc.
    std::vector<Note> result;
    const Note bass(root, 2);
    result.push_back(bass);

    for (size_t i = 0; i < notes.size(); ++i)
    {
        int oct = (i < 2) ? 3 : 4;
        result.push_back(notes[i].withOctave(oct));
    }
    return result;
}

std::vector<Note> Voicing::makePad(const std::vector<Note>& notes)
{
    // Pad: Wide lush spread across 3 octaves
    std::vector<Note> result;
    for (const auto& n : notes)
    {
        result.push_back(n.transposedBy(-12)); // Low foundation
        result.push_back(n);                  // Center body
    }
    return result;
}

std::vector<Note> Voicing::makeBassPlusChord(const std::vector<Note>& notes, PitchClass root)
{
    // Separate deep sub-bass note (Octave 1 or 2) + chord block in Octave 4
    std::vector<Note> result;
    result.push_back(Note(root, 2));

    for (const auto& n : notes)
    {
        int oct = std::max(n.getOctave(), 4);
        result.push_back(n.withOctave(oct));
    }
    return result;
}

std::string_view Voicing::getStyleName(VoicingStyle style)
{
    switch (style)
    {
        case VoicingStyle::Close:          return "Close";
        case VoicingStyle::Open:           return "Open";
        case VoicingStyle::Wide:           return "Wide";
        case VoicingStyle::Drop2:          return "Drop 2";
        case VoicingStyle::Piano:          return "Piano";
        case VoicingStyle::Guitar:         return "Guitar";
        case VoicingStyle::Pad:            return "Pad";
        case VoicingStyle::BassPlusChord:  return "Bass + Chord";
    }
    return "Close";
}

std::vector<VoicingStyle> Voicing::getAllStyles()
{
    return {
        VoicingStyle::Close,
        VoicingStyle::Open,
        VoicingStyle::Wide,
        VoicingStyle::Drop2,
        VoicingStyle::Piano,
        VoicingStyle::Guitar,
        VoicingStyle::Pad,
        VoicingStyle::BassPlusChord
    };
}

} // namespace ChordForge::Music
