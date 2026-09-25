#include "VoiceLeading.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace ChordForge::Music
{

float VoiceLeading::calculateCost(
    const std::vector<Note>& current,
    const std::vector<Note>& candidate,
    const VoiceLeadingConfig& config)
{
    if (current.empty() || candidate.empty()) return 0.0f;

    float totalDistance = 0.0f;
    float maxJump = 0.0f;

    const size_t count = std::min(current.size(), candidate.size());
    for (size_t i = 0; i < count; ++i)
    {
        float diff = static_cast<float>(std::abs(candidate[i].getMidiNumber() - current[i].getMidiNumber()));
        totalDistance += diff;
        if (diff > maxJump) maxJump = diff;
    }

    // Penalize notes outside of range
    float rangePenalty = 0.0f;
    for (const auto& n : candidate)
    {
        int p = n.getMidiNumber();
        if (p < config.minPitch) rangePenalty += static_cast<float>(config.minPitch - p) * 3.0f;
        if (p > config.maxPitch) rangePenalty += static_cast<float>(p - config.maxPitch) * 3.0f;
    }

    // Penalize excessive voice spacing (> 19 semitones between adjacent voices, except bass)
    float spacingPenalty = 0.0f;
    for (size_t i = 1; i < candidate.size(); ++i)
    {
        int space = candidate[i].getMidiNumber() - candidate[i - 1].getMidiNumber();
        if (space > 19)
        {
            spacingPenalty += static_cast<float>(space - 19) * config.weightSpacing;
        }
    }

    return (totalDistance * config.weightTotalDistance) +
           (maxJump * config.weightMaxJump) +
           rangePenalty +
           spacingPenalty;
}

std::vector<Note> VoiceLeading::findOptimalVoicing(
    const std::vector<Note>& previousVoicing,
    const Chord& targetChord,
    VoicingStyle style,
    const VoiceLeadingConfig& config)
{
    if (previousVoicing.empty())
    {
        VoicingOptions defaultOpts;
        defaultOpts.style = style;
        return Voicing::generate(targetChord, defaultOpts);
    }

    float bestCost = std::numeric_limits<float>::max();
    std::vector<Note> bestVoicing;

    const int maxInversion = static_cast<int>(targetChord.getNoteCount());

    // Search permutations across inversions (0..maxInversion) and octave shifts (-1..+1)
    for (int inv = 0; inv < maxInversion; ++inv)
    {
        for (int octShift = -1; octShift <= 1; ++octShift)
        {
            VoicingOptions opts;
            opts.style = style;
            opts.inversion = inv;
            opts.octaveShift = octShift;

            auto candidate = Voicing::generate(targetChord, opts);
            float cost = calculateCost(previousVoicing, candidate, config);

            if (cost < bestCost)
            {
                bestCost = cost;
                bestVoicing = std::move(candidate);
            }
        }
    }

    return bestVoicing.empty() ? Voicing::generate(targetChord, VoicingOptions{}) : bestVoicing;
}

std::vector<std::vector<Note>> VoiceLeading::smoothProgression(
    const std::vector<Chord>& chords,
    const VoicingOptions& baseOptions,
    const VoiceLeadingConfig& config)
{
    if (chords.empty()) return {};

    std::vector<std::vector<Note>> result;
    result.reserve(chords.size());

    // First chord uses default/base voicing
    result.push_back(Voicing::generate(chords[0], baseOptions));

    // Subsequent chords are voice-lead from the preceding chord
    for (size_t i = 1; i < chords.size(); ++i)
    {
        auto nextVoicing = findOptimalVoicing(result[i - 1], chords[i], baseOptions.style, config);
        result.push_back(std::move(nextVoicing));
    }

    return result;
}

} // namespace ChordForge::Music
