#include "MidiExporter.h"
#include <juce_core/juce_core.h>

namespace ChordForge::Midi
{

juce::MidiFile MidiExporter::createMidiFile(
    const Music::Progression& progression,
    int ppqResolution,
    int midiChannel,
    uint8_t velocity)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ppqResolution);

    juce::MidiMessageSequence trackSequence;

    // Tempo meta event
    const double bpm = progression.getBpm();
    trackSequence.addEvent(juce::MidiMessage::tempoMetaEvent(static_cast<int>(60000000.0 / bpm)), 0.0);

    // Time signature meta event (4/4)
    trackSequence.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0.0);

    // Track name
    trackSequence.addEvent(juce::MidiMessage::textMetaEvent(3, "ChordForge Progression"), 0.0);

    const auto voicings = progression.generateAllVoicings();
    const auto& slots = progression.getSlots();

    double currentPpqTime = 0.0;

    for (size_t i = 0; i < voicings.size() && i < slots.size(); ++i)
    {
        const auto& notes = voicings[i];
        const double durationBeats = slots[i].durationBeats;
        const double noteLengthPpq = durationBeats * static_cast<double>(ppqResolution);

        // Add Note On events
        for (const auto& n : notes)
        {
            trackSequence.addEvent(
                juce::MidiMessage::noteOn(midiChannel, n.getMidiNumber(), velocity),
                currentPpqTime
            );
        }

        // Add Note Off events (subtle 5-tick gate gap for natural legato/re-triggering)
        const double noteOffTime = currentPpqTime + std::max(noteLengthPpq - 5.0, 1.0);
        for (const auto& n : notes)
        {
            trackSequence.addEvent(
                juce::MidiMessage::noteOff(midiChannel, n.getMidiNumber(), (uint8_t)0),
                noteOffTime
            );
        }

        currentPpqTime += noteLengthPpq;
    }

    // End of track meta event
    trackSequence.addEvent(juce::MidiMessage::endOfTrack(), currentPpqTime);

    midiFile.addTrack(trackSequence);
    return midiFile;
}

bool MidiExporter::exportToFile(
    const Music::Progression& progression,
    const std::string& destinationFilePath,
    int ppqResolution,
    int midiChannel)
{
    juce::MidiFile midiFile = createMidiFile(progression, ppqResolution, midiChannel);
    juce::File targetFile(juce::String::fromUTF8(destinationFilePath.c_str()));

    if (targetFile.existsAsFile())
    {
        targetFile.deleteFile();
    }

    juce::FileOutputStream stream(targetFile);
    if (!stream.openedOk())
    {
        return false;
    }

    return midiFile.writeTo(stream, 1);
}

std::vector<uint8_t> MidiExporter::getMidiFileBytes(
    const Music::Progression& progression,
    int ppqResolution,
    int midiChannel)
{
    juce::MidiFile midiFile = createMidiFile(progression, ppqResolution, midiChannel);
    juce::MemoryOutputStream memStream;
    midiFile.writeTo(memStream, 1);

    const size_t dataSize = memStream.getDataSize();
    const uint8_t* rawData = static_cast<const uint8_t*>(memStream.getData());

    return std::vector<uint8_t>(rawData, rawData + dataSize);
}

} // namespace ChordForge::Midi
