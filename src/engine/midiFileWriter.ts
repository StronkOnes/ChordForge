import { ProgressionSlotState } from '../types/music';
import { generateVoicingNotes, findOptimalVoiceLeading } from './musicTheory';

// Helper: encode variable length quantity (VLQ)
function writeVLQ(value: number): number[] {
  let buffer = value & 0x7f;
  const bytes: number[] = [];
  while ((value >>= 7) > 0) {
    buffer <<= 8;
    buffer |= 0x80;
    buffer += value & 0x7f;
  }
  while (true) {
    bytes.push(buffer & 0xff);
    if (buffer & 0x80) buffer >>= 8;
    else break;
  }
  return bytes;
}

function writeString(str: string): number[] {
  const bytes: number[] = [];
  for (let i = 0; i < str.length; i++) {
    bytes.push(str.charCodeAt(i));
  }
  return bytes;
}

function write32Bit(val: number): number[] {
  return [(val >> 24) & 0xff, (val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff];
}

function write16Bit(val: number): number[] {
  return [(val >> 8) & 0xff, val & 0xff];
}

export function generateStandardMidiFile(
  slots: ProgressionSlotState[],
  bpm = 120,
  enableVoiceLeading = true,
  ppq = 960,
): Uint8Array {
  const trackBytes: number[] = [];

  // 1. Meta Event: Set Tempo (microsec per quarter note = 60,000,000 / BPM)
  const mpqn = Math.round(60000000 / bpm);
  trackBytes.push(0x00, 0xff, 0x51, 0x03, (mpqn >> 16) & 0xff, (mpqn >> 8) & 0xff, mpqn & 0xff);

  // 2. Meta Event: Time Signature (4/4)
  trackBytes.push(0x00, 0xff, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08);

  // 3. Meta Event: Track Name
  const trackName = writeString('ChordForge Progression');
  trackBytes.push(0x00, 0xff, 0x03, trackName.length, ...trackName);

  // 4. Generate voiced notes for each slot
  const allVoicings: number[][] = [];
  let prevVoicingNotes: ReturnType<typeof generateVoicingNotes> = [];

  for (let i = 0; i < slots.length; i++) {
    const slot = slots[i];
    let notes: ReturnType<typeof generateVoicingNotes>;
    if (enableVoiceLeading && i > 0) {
      notes = findOptimalVoiceLeading(prevVoicingNotes, slot.chord, slot.voicingStyle);
    } else {
      notes = generateVoicingNotes(slot.chord, slot.voicingStyle, slot.inversion, slot.octaveShift);
    }
    prevVoicingNotes = notes;
    allVoicings.push(notes.map((n) => n.midi));
  }

  // 5. Append Note On and Note Off events
  for (let i = 0; i < slots.length; i++) {
    const chordMidis = allVoicings[i];
    const durationBeats = slots[i].durationBeats || 4.0;
    const durationTicks = Math.round(durationBeats * ppq);
    const gateTicks = Math.max(1, durationTicks - 10); // subtle gap before next chord

    // Note Ons (all at delta time 0, except first note of slot which connects to previous end)
    for (let nIdx = 0; nIdx < chordMidis.length; nIdx++) {
      const delta = writeVLQ(0);
      trackBytes.push(...delta, 0x90, chordMidis[nIdx] & 0x7f, 96);
    }

    // Note Offs
    for (let nIdx = 0; nIdx < chordMidis.length; nIdx++) {
      // First note off event carries the chord duration delta time; subsequent note offs carry delta 0
      const delta = writeVLQ(nIdx === 0 ? gateTicks : 0);
      trackBytes.push(...delta, 0x80, chordMidis[nIdx] & 0x7f, 0x00);
    }

    // Tail delta for gap
    const tailDelta = writeVLQ(10);
    trackBytes.push(...tailDelta, 0xb0, 0x7b, 0x00); // CC 123 all notes off / harmless placeholder
  }

  // End of Track meta event
  trackBytes.push(0x00, 0xff, 0x2f, 0x00);

  // Assemble full MIDI file
  const fullFile: number[] = [
    // Header Chunk 'MThd'
    0x4d, 0x54, 0x68, 0x64,
    ...write32Bit(6), // Header size (always 6)
    ...write16Bit(0), // Format 0 (single multi-channel track)
    ...write16Bit(1), // 1 track
    ...write16Bit(ppq), // PPQ

    // Track Chunk 'MTrk'
    0x4d, 0x54, 0x72, 0x6b,
    ...write32Bit(trackBytes.length),
    ...trackBytes,
  ];

  return new Uint8Array(fullFile);
}

export function downloadMidiFile(
  slots: ProgressionSlotState[],
  bpm: number,
  enableVoiceLeading: boolean,
  fileName = 'ChordForge_Progression.mid',
) {
  const bytes = generateStandardMidiFile(slots, bpm, enableVoiceLeading);
  const blob = new Blob([bytes.buffer as ArrayBuffer], { type: 'audio/midi' });
  const url = URL.createObjectURL(blob);

  const a = document.createElement('a');
  a.href = url;
  a.download = fileName;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}
