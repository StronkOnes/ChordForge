#pragma once

#include "Chord.h"
#include "Note.h"
#include "Voicing.h"
#include <vector>

namespace ChordForge::Music
{

struct VoiceLeadingConfig
{
    int minPitch{36}; // C2
    int maxPitch{84}; // C6
    float weightTotalDistance{1.0f};
    float weightMaxJump{1.5f};
    float weightCrossing{2.0f};
    float weightSpacing{0.8f};
};

class VoiceLeading
{
public:
    // Given previous chord notes, find the optimal inversion/octave voicing for target chord
    static std::vector<Note> findOptimalVoicing(
        const std::vector<Note>& previousVoicing,
        const Chord& targetChord,
        VoicingStyle style = VoicingStyle::Close,
        const VoiceLeadingConfig& config = VoiceLeadingConfig{}
    );

    // Calculate penalty/cost between two sets of voiced notes
    static float calculateCost(
        const std::vector<Note>& current,
        const std::vector<Note>& candidate,
        const VoiceLeadingConfig& config = VoiceLeadingConfig{}
    );

    // Apply smooth voice leading across an entire progression of chords
    static std::vector<std::vector<Note>> smoothProgression(
        const std::vector<Chord>& chords,
        const VoicingOptions& baseOptions,
        const VoiceLeadingConfig& config = VoiceLeadingConfig{}
    );
};

} // namespace ChordForge::Music
