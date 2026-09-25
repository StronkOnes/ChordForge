import {
  PitchClassName,
  ScaleTypeName,
  VoicingStyleName,
  NoteInfo,
  ChordDefinition,
  RandomizerStyleName,
} from '../types/music';

export const PITCH_CLASSES: PitchClassName[] = [
  'C',
  'C#',
  'D',
  'D#',
  'E',
  'F',
  'F#',
  'G',
  'G#',
  'A',
  'A#',
  'B',
];

export const SCALE_INTERVALS: Record<ScaleTypeName, number[]> = {
  'Major': [0, 2, 4, 5, 7, 9, 11],
  'Natural Minor': [0, 2, 3, 5, 7, 8, 10],
  'Dorian': [0, 2, 3, 5, 7, 9, 10],
  'Phrygian': [0, 1, 3, 5, 7, 8, 10],
  'Lydian': [0, 2, 4, 6, 7, 9, 11],
  'Mixolydian': [0, 2, 4, 5, 7, 9, 10],
  'Locrian': [0, 1, 3, 5, 6, 8, 10],
  'Harmonic Minor': [0, 2, 3, 5, 7, 8, 11],
  'Melodic Minor': [0, 2, 3, 5, 7, 9, 11],
};

export const VOICING_STYLES: VoicingStyleName[] = [
  'Close',
  'Open',
  'Wide',
  'Drop 2',
  'Piano',
  'Guitar',
  'Pad',
  'Bass + Chord',
];

export function getMidiNote(pitchClass: PitchClassName, octave: number): number {
  const pc = PITCH_CLASSES.indexOf(pitchClass);
  return Math.max(0, Math.min(127, (octave + 1) * 12 + pc));
}

export function midiToNoteInfo(midi: number): NoteInfo {
  const clamped = Math.max(0, Math.min(127, Math.round(midi)));
  const pcIndex = (clamped % 12 + 12) % 12;
  const octave = Math.floor(clamped / 12) - 1;
  const pc = PITCH_CLASSES[pcIndex];
  const freq = 440 * Math.pow(2, (clamped - 69) / 12);
  return {
    midi: clamped,
    pitchClass: pc,
    octave,
    name: `${pc}${octave}`,
    frequencyHz: freq,
  };
}

export function getScaleNoteAtDegree(
  root: PitchClassName,
  scaleType: ScaleTypeName,
  degree: number, // 1-based (1..7)
  baseOctave = 4,
): NoteInfo {
  const intervals = SCALE_INTERVALS[scaleType];
  const numDegrees = intervals.length;

  let index = degree - 1;
  let octaveShift = 0;

  while (index < 0) {
    index += numDegrees;
    octaveShift -= 1;
  }
  while (index >= numDegrees) {
    index -= numDegrees;
    octaveShift += 1;
  }

  const rootMidi = getMidiNote(root, baseOctave + octaveShift);
  return midiToNoteInfo(rootMidi + intervals[index]);
}

export function identifyQualityAndRoman(
  degree: number,
  intervals: number[],
  includeSeventh: boolean,
): { quality: string; roman: string } {
  const third = intervals[1] % 12;
  const fifth = intervals[2] % 12;
  const seventh = includeSeventh && intervals.length >= 4 ? intervals[3] % 12 : -1;

  const UPPER = ['I', 'II', 'III', 'IV', 'V', 'VI', 'VII'];
  const LOWER = ['i', 'ii', 'iii', 'iv', 'v', 'vi', 'vii'];
  const idx = Math.max(0, Math.min(6, degree - 1));

  if (!includeSeventh) {
    if (third === 4 && fifth === 7) return { quality: 'Major', roman: UPPER[idx] };
    if (third === 3 && fifth === 7) return { quality: 'Minor', roman: LOWER[idx] };
    if (third === 3 && fifth === 6) return { quality: 'Diminished', roman: `${LOWER[idx]}°` };
    if (third === 4 && fifth === 8) return { quality: 'Augmented', roman: `${UPPER[idx]}+` };
    return { quality: 'Major', roman: UPPER[idx] };
  } else {
    if (third === 4 && fifth === 7 && seventh === 11) return { quality: 'Major 7', roman: `${UPPER[idx]}maj7` };
    if (third === 3 && fifth === 7 && seventh === 10) return { quality: 'Minor 7', roman: `${LOWER[idx]}7` };
    if (third === 4 && fifth === 7 && seventh === 10) return { quality: 'Dominant 7', roman: `${UPPER[idx]}7` };
    if (third === 3 && fifth === 6 && seventh === 9) return { quality: 'Diminished 7', roman: `${LOWER[idx]}°7` };
    if (third === 3 && fifth === 6 && seventh === 10) return { quality: 'Half-Diminished 7', roman: `${LOWER[idx]}ø7` };
    if (third === 3 && fifth === 7 && seventh === 11) return { quality: 'Minor-Major 7', roman: `${LOWER[idx]}m(maj7)` };
    return { quality: '7th', roman: `${UPPER[idx]}7` };
  }
}

export function buildDiatonicChord(
  key: PitchClassName,
  scaleType: ScaleTypeName,
  degree: number,
  includeSeventh: boolean,
  octave = 4,
): ChordDefinition {
  const rootNote = getScaleNoteAtDegree(key, scaleType, degree, octave);
  const thirdNote = getScaleNoteAtDegree(key, scaleType, degree + 2, octave);
  const fifthNote = getScaleNoteAtDegree(key, scaleType, degree + 4, octave);

  const rootMidi = rootNote.midi;
  const intervals = [0, thirdNote.midi - rootMidi, fifthNote.midi - rootMidi];

  if (includeSeventh) {
    const seventhNote = getScaleNoteAtDegree(key, scaleType, degree + 6, octave);
    intervals.push(seventhNote.midi - rootMidi);
  }

  const { quality, roman } = identifyQualityAndRoman(degree, intervals, includeSeventh);

  return {
    root: rootNote.pitchClass,
    qualityName: quality,
    romanNumeral: roman,
    degree,
    intervals,
    octave,
    hasSeventh: includeSeventh,
  };
}

export function applyInversion(notes: number[], inversion: number): number[] {
  if (notes.length === 0) return [];
  const result = [...notes];
  const count = result.length;
  const effective = ((inversion % count) + count) % count;

  for (let i = 0; i < effective; i++) {
    const lowest = result.shift()!;
    result.push(lowest + 12);
  }
  return result;
}

export function generateVoicingNotes(
  chord: ChordDefinition,
  style: VoicingStyleName,
  inversion: number,
  octaveShift: number,
): NoteInfo[] {
  const baseRootMidi = getMidiNote(chord.root, chord.octave);
  const baseNotes = chord.intervals.map((interval) => baseRootMidi + interval);

  // Inversion
  const inverted = applyInversion(baseNotes, inversion);

  // Style transformations
  let styled: number[] = [];
  switch (style) {
    case 'Close':
      styled = [...inverted];
      break;

    case 'Open': {
      styled = [...inverted];
      if (styled.length >= 3) {
        styled[1] += 12; // Spread middle voice up an octave
      }
      break;
    }

    case 'Wide': {
      styled = [...inverted];
      if (styled.length >= 1) styled[0] -= 12;
      if (styled.length >= 3) styled[styled.length - 1] += 12;
      break;
    }

    case 'Drop 2': {
      styled = [...inverted].sort((a, b) => a - b);
      if (styled.length >= 3) {
        const dropIndex = styled.length - 2;
        styled[dropIndex] -= 12;
      }
      break;
    }

    case 'Piano': {
      // Left hand bass note (octave 2) + right hand chord (octave 4)
      const bassMidi = getMidiNote(chord.root, 2);
      styled = [bassMidi];
      for (const n of inverted) {
        const info = midiToNoteInfo(n);
        styled.push(getMidiNote(info.pitchClass, 4));
      }
      break;
    }

    case 'Guitar': {
      const bassMidi = getMidiNote(chord.root, 2);
      styled = [bassMidi];
      for (let i = 0; i < inverted.length; i++) {
        const info = midiToNoteInfo(inverted[i]);
        styled.push(getMidiNote(info.pitchClass, i < 2 ? 3 : 4));
      }
      break;
    }

    case 'Pad': {
      styled = [];
      for (const n of inverted) {
        styled.push(n - 12);
        styled.push(n);
      }
      break;
    }

    case 'Bass + Chord': {
      const subBass = getMidiNote(chord.root, 2);
      styled = [subBass];
      for (const n of inverted) {
        const info = midiToNoteInfo(n);
        styled.push(getMidiNote(info.pitchClass, 4));
      }
      break;
    }
  }

  // Shift & Clamp
  const totalShift = octaveShift * 12;
  const sortedUniqueMidis = Array.from(
    new Set(
      styled
        .map((m) => Math.max(0, Math.min(127, m + totalShift)))
        .sort((a, b) => a - b),
    ),
  );

  return sortedUniqueMidis.map((m) => midiToNoteInfo(m));
}

export function calculateVoiceLeadingCost(prevNotes: number[], candidateNotes: number[]): number {
  if (prevNotes.length === 0 || candidateNotes.length === 0) return 0;

  let totalDistance = 0;
  let maxJump = 0;
  const count = Math.min(prevNotes.length, candidateNotes.length);

  for (let i = 0; i < count; i++) {
    const diff = Math.abs(candidateNotes[i] - prevNotes[i]);
    totalDistance += diff;
    if (diff > maxJump) maxJump = diff;
  }

  let rangePenalty = 0;
  for (const p of candidateNotes) {
    if (p < 36) rangePenalty += (36 - p) * 3;
    if (p > 84) rangePenalty += (p - 84) * 3;
  }

  return totalDistance + maxJump * 1.5 + rangePenalty;
}

export function findOptimalVoiceLeading(
  prevNotes: NoteInfo[],
  targetChord: ChordDefinition,
  style: VoicingStyleName,
): NoteInfo[] {
  if (prevNotes.length === 0) {
    return generateVoicingNotes(targetChord, style, 0, 0);
  }

  const prevMidis = prevNotes.map((n) => n.midi);
  let bestCost = Infinity;
  let bestVoicing: NoteInfo[] = [];

  const maxInversions = targetChord.intervals.length;

  for (let inv = 0; inv < maxInversions; inv++) {
    for (let octShift = -1; octShift <= 1; octShift++) {
      const candidate = generateVoicingNotes(targetChord, style, inv, octShift);
      const candidateMidis = candidate.map((n) => n.midi);
      const cost = calculateVoiceLeadingCost(prevMidis, candidateMidis);

      if (cost < bestCost) {
        bestCost = cost;
        bestVoicing = candidate;
      }
    }
  }

  return bestVoicing.length > 0 ? bestVoicing : generateVoicingNotes(targetChord, style, 0, 0);
}

export function getRandomProgression(
  key: PitchClassName,
  scaleType: ScaleTypeName,
  style: RandomizerStyleName,
  slotCount = 4,
): { degree: number; hasSeventh: boolean }[] {
  const POP_PATTERNS = [
    { degrees: [1, 5, 6, 4], has7th: false },
    { degrees: [6, 4, 1, 5], has7th: false },
    { degrees: [1, 4, 6, 5], has7th: false },
    { degrees: [1, 6, 4, 5], has7th: false },
    { degrees: [1, 4, 5, 4], has7th: false },
  ];

  const JAZZ_PATTERNS = [
    { degrees: [2, 5, 1, 6], has7th: true },
    { degrees: [1, 6, 2, 5], has7th: true },
    { degrees: [3, 6, 2, 5], has7th: true },
  ];

  const MINOR_PATTERNS = [
    { degrees: [1, 6, 3, 7], has7th: false },
    { degrees: [1, 4, 5, 1], has7th: false },
    { degrees: [1, 6, 7, 1], has7th: false },
  ];

  const RNB_PATTERNS = [
    { degrees: [4, 3, 6, 2], has7th: true },
    { degrees: [1, 6, 2, 5], has7th: true },
    { degrees: [4, 5, 3, 6], has7th: true },
  ];

  let chosenPool = POP_PATTERNS;
  if (style === 'Jazz') chosenPool = JAZZ_PATTERNS;
  else if (style === 'Minor/Sad') chosenPool = MINOR_PATTERNS;
  else if (style === 'R&B / Soul') chosenPool = RNB_PATTERNS;

  const chosen = chosenPool[Math.floor(Math.random() * chosenPool.length)];
  const result: { degree: number; hasSeventh: boolean }[] = [];

  for (let i = 0; i < slotCount; i++) {
    const deg = chosen.degrees[i % chosen.degrees.length];
    result.push({ degree: deg, hasSeventh: chosen.has7th });
  }

  return result;
}
