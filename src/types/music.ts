export type PitchClassName =
  | 'C'
  | 'C#'
  | 'D'
  | 'D#'
  | 'E'
  | 'F'
  | 'F#'
  | 'G'
  | 'G#'
  | 'A'
  | 'A#'
  | 'B';

export type ScaleTypeName =
  | 'Major'
  | 'Natural Minor'
  | 'Dorian'
  | 'Phrygian'
  | 'Lydian'
  | 'Mixolydian'
  | 'Locrian'
  | 'Harmonic Minor'
  | 'Melodic Minor';

export type VoicingStyleName =
  | 'Close'
  | 'Open'
  | 'Wide'
  | 'Drop 2'
  | 'Piano'
  | 'Guitar'
  | 'Pad'
  | 'Bass + Chord';

export type RandomizerStyleName =
  | 'Pop'
  | 'Jazz'
  | 'Minor/Sad'
  | 'R&B / Soul'
  | 'EDM Modern'
  | 'Ambient Chill';

export type TriggerModeName =
  | 'Host DAW Sequencer'
  | 'Root Trigger'
  | 'Scale Degree Trigger'
  | 'Progression Slot Trigger';

export interface NoteInfo {
  midi: number;
  pitchClass: PitchClassName;
  octave: number;
  name: string; // e.g. "C4"
  frequencyHz: number;
}

export interface ChordDefinition {
  root: PitchClassName;
  qualityName: string;
  romanNumeral: string;
  degree: number;
  intervals: number[];
  octave: number;
  hasSeventh: boolean;
}

export interface ProgressionSlotState {
  id: string;
  degree: number;
  chord: ChordDefinition;
  voicingStyle: VoicingStyleName;
  inversion: number;
  octaveShift: number;
  isLocked: boolean;
  durationBeats: number;
}

export interface PresetDef {
  name: string;
  category: string;
  key: PitchClassName;
  scale: ScaleTypeName;
  degrees: number[];
  hasSevenths: boolean;
  voicing: VoicingStyleName;
  bpm: number;
}
