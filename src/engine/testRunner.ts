import {
  getMidiNote,
  midiToNoteInfo,
  buildDiatonicChord,
  generateVoicingNotes,
  findOptimalVoiceLeading,
  calculateVoiceLeadingCost,
  SCALE_INTERVALS,
} from './musicTheory';
import { generateStandardMidiFile } from './midiFileWriter';

export interface TestResult {
  category: string;
  name: string;
  passed: boolean;
  expected: string;
  actual: string;
  details?: string;
}

export function runBrowserMusicTheoryTests(): {
  results: TestResult[];
  passedCount: number;
  failedCount: number;
} {
  const results: TestResult[] = [];

  const expectEq = (
    category: string,
    name: string,
    actual: unknown,
    expected: unknown,
    details?: string,
  ) => {
    const passed = JSON.stringify(actual) === JSON.stringify(expected);
    results.push({
      category,
      name,
      passed,
      expected: String(expected),
      actual: String(actual),
      details,
    });
  };

  const expectTrue = (
    category: string,
    name: string,
    cond: boolean,
    details?: string,
  ) => {
    results.push({
      category,
      name,
      passed: cond,
      expected: 'true',
      actual: String(cond),
      details,
    });
  };

  // 1. Note representation
  expectEq('Notes & Pitch Classes', 'C4 MIDI Number', getMidiNote('C', 4), 60);
  expectEq('Notes & Pitch Classes', 'A4 MIDI Number', getMidiNote('A', 4), 69);
  expectEq('Notes & Pitch Classes', 'F#3 MIDI Number', getMidiNote('F#', 3), 54);
  const a4Info = midiToNoteInfo(69);
  expectTrue(
    'Notes & Pitch Classes',
    'A4 Tuning ~440Hz',
    Math.abs(a4Info.frequencyHz - 440) < 0.05,
    `${a4Info.frequencyHz.toFixed(2)} Hz`,
  );

  // 2. Scales
  expectEq('Scales', 'C Major Intervals', SCALE_INTERVALS['Major'], [0, 2, 4, 5, 7, 9, 11]);
  expectEq('Scales', 'A Minor Intervals', SCALE_INTERVALS['Natural Minor'], [0, 2, 3, 5, 7, 8, 10]);

  // 3. Section 30 Diatonic Chords in C Major
  // I = C E G
  const I = buildDiatonicChord('C', 'Major', 1, false, 4);
  const nI = generateVoicingNotes(I, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'I = C E G (C4 E4 G4)', nI.join(' '), 'C4 E4 G4');

  // ii = D F A
  const ii = buildDiatonicChord('C', 'Major', 2, false, 4);
  const nII = generateVoicingNotes(ii, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'ii = D F A (D4 F4 A4)', nII.join(' '), 'D4 F4 A4');

  // iii = E G B
  const iii = buildDiatonicChord('C', 'Major', 3, false, 4);
  const nIII = generateVoicingNotes(iii, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'iii = E G B (E4 G4 B4)', nIII.join(' '), 'E4 G4 B4');

  // IV = F A C
  const IV = buildDiatonicChord('C', 'Major', 4, false, 4);
  const nIV = generateVoicingNotes(IV, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'IV = F A C (F4 A4 C5)', nIV.join(' '), 'F4 A4 C5');

  // V = G B D
  const V = buildDiatonicChord('C', 'Major', 5, false, 4);
  const nV = generateVoicingNotes(V, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'V = G B D (G4 B4 D5)', nV.join(' '), 'G4 B4 D5');

  // vi = A C E
  const vi = buildDiatonicChord('C', 'Major', 6, false, 4);
  const nVI = generateVoicingNotes(vi, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'vi = A C E (A4 C5 E5)', nVI.join(' '), 'A4 C5 E5');

  // vii° = B D F
  const vii = buildDiatonicChord('C', 'Major', 7, false, 4);
  const nVII = generateVoicingNotes(vii, 'Close', 0, 0).map((n) => n.name);
  expectEq('C Major Diatonic Triads', 'vii° = B D F (B4 D5 F5)', nVII.join(' '), 'B4 D5 F5');

  // 4. Inversions
  const rootInv = generateVoicingNotes(I, 'Close', 0, 0).map((n) => n.name).join(' ');
  const inv1 = generateVoicingNotes(I, 'Close', 1, 0).map((n) => n.name).join(' ');
  const inv2 = generateVoicingNotes(I, 'Close', 2, 0).map((n) => n.name).join(' ');
  expectEq('Inversions', 'Root Position (C E G)', rootInv, 'C4 E4 G4');
  expectEq('Inversions', '1st Inversion (E G C)', inv1, 'E4 G4 C5');
  expectEq('Inversions', '2nd Inversion (G C E)', inv2, 'G4 C5 E5');

  // 5. Seventh Chords
  const Imaj7 = buildDiatonicChord('C', 'Major', 1, true, 4);
  const nImaj7 = generateVoicingNotes(Imaj7, 'Close', 0, 0).map((n) => n.name).join(' ');
  expectEq('7th Chords', 'Imaj7 (C E G B)', nImaj7, 'C4 E4 G4 B4');

  const V7 = buildDiatonicChord('C', 'Major', 5, true, 4);
  const nV7 = generateVoicingNotes(V7, 'Close', 0, 0).map((n) => n.name).join(' ');
  expectEq('7th Chords', 'V7 (G B D F)', nV7, 'G4 B4 D5 F5');

  // 6. Voice Leading
  const voicedC = generateVoicingNotes(I, 'Close', 0, 0);
  const optimalG = findOptimalVoiceLeading(voicedC, V, 'Close');
  const unguidedG = generateVoicingNotes(V, 'Close', 0, 0);
  const costOpt = calculateVoiceLeadingCost(voicedC.map((n) => n.midi), optimalG.map((n) => n.midi));
  const costUnguided = calculateVoiceLeadingCost(voicedC.map((n) => n.midi), unguidedG.map((n) => n.midi));
  expectTrue(
    'Voice Leading Engine',
    'Optimal Voice Leading minimizes pitch jump (Cost <= Unguided)',
    costOpt <= costUnguided,
    `Optimal Cost: ${costOpt} <= Raw Cost: ${costUnguided}`,
  );

  // 7. MIDI File Generation
  const testSlot = {
    id: 'test-slot',
    degree: 1,
    chord: I,
    voicingStyle: 'Close' as const,
    inversion: 0,
    octaveShift: 0,
    isLocked: false,
    durationBeats: 4,
  };
  const midiBytes = generateStandardMidiFile([testSlot], 120, false, 960);
  // Check 'MThd' ASCII header
  const isMThd =
    midiBytes[0] === 0x4d &&
    midiBytes[1] === 0x54 &&
    midiBytes[2] === 0x68 &&
    midiBytes[3] === 0x64;
  expectTrue('Standard MIDI File Writer', 'Valid MThd Header Chunk in Binary', isMThd);
  expectTrue('Standard MIDI File Writer', 'Non-empty valid MIDI byte payload', midiBytes.length > 30);

  const passedCount = results.filter((r) => r.passed).length;
  const failedCount = results.filter((r) => !r.passed).length;

  return { results, passedCount, failedCount };
}
