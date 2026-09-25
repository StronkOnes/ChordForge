import React, { useState, useEffect, useRef, useCallback } from 'react';
import {
  PitchClassName,
  ScaleTypeName,
  VoicingStyleName,
  ProgressionSlotState,
  RandomizerStyleName,
  NoteInfo,
} from '../types/music';
import {
  PITCH_CLASSES,
  SCALE_INTERVALS,
  VOICING_STYLES,
  buildDiatonicChord,
  generateVoicingNotes,
  findOptimalVoiceLeading,
  getRandomProgression,
} from '../engine/musicTheory';
import { audioSynth } from '../engine/audioSynth';
import { downloadMidiFile } from '../engine/midiFileWriter';
import { PianoVisualizer } from './PianoVisualizer';
import {
  Play,
  Square,
  Download,
  Lock,
  Unlock,
  Shuffle,
  Volume2,
  VolumeX,
  Plus,
  Trash2,
  SlidersHorizontal,
  ChevronDown,
} from 'lucide-react';

interface HardwarePluginViewProps {
  onOpenDawGuide: () => void;
}

export const HardwarePluginView: React.FC<HardwarePluginViewProps> = ({ onOpenDawGuide }) => {
  // Master tone state
  const [selectedKey, setSelectedKey] = useState<PitchClassName>('C');
  const [selectedScale, setSelectedScale] = useState<ScaleTypeName>('Major');
  const [selectedDegree, setSelectedDegree] = useState<number>(1);
  const [includeSeventh, setIncludeSeventh] = useState<boolean>(false);
  const [voicingStyle, setVoicingStyle] = useState<VoicingStyleName>('Close');
  const [inversion, setInversion] = useState<number>(0);
  const [octaveShift, setOctaveShift] = useState<number>(0);
  const [enableVoiceLeading, setEnableVoiceLeading] = useState<boolean>(true);
  const [bpm, setBpm] = useState<number>(120);
  const [randomizerStyle, setRandomizerStyle] = useState<RandomizerStyleName>('Pop');
  const [isMuted, setIsMuted] = useState<boolean>(false);

  // Progression Slots (initial 4 classic chords: I - V - vi - IV)
  const [slots, setSlots] = useState<ProgressionSlotState[]>(() => {
    const initialDegrees = [1, 5, 6, 4];
    return initialDegrees.map((deg, idx) => {
      const chord = buildDiatonicChord('C', 'Major', deg, false, 4);
      return {
        id: `slot-${idx}-${deg}`,
        degree: deg,
        chord,
        voicingStyle: 'Close',
        inversion: 0,
        octaveShift: 0,
        isLocked: false,
        durationBeats: 4,
      };
    });
  });

  // Playback / Sequencer state
  const [isPlaying, setIsPlaying] = useState<boolean>(false);
  const [activeSlotIndex, setActiveSlotIndex] = useState<number>(-1);
  const [currentVoicedNotes, setCurrentVoicedNotes] = useState<NoteInfo[]>([]);

  // Active current edited chord preview
  const currentChordDef = buildDiatonicChord(
    selectedKey,
    selectedScale,
    selectedDegree,
    includeSeventh,
    4,
  );

  // Update preview voiced notes when single chord controls change
  useEffect(() => {
    if (!isPlaying) {
      const notes = generateVoicingNotes(
        currentChordDef,
        voicingStyle,
        inversion,
        octaveShift,
      );
      setCurrentVoicedNotes(notes);
    }
  }, [selectedKey, selectedScale, selectedDegree, includeSeventh, voicingStyle, inversion, octaveShift, isPlaying]);

  // When Key or Scale changes, update unlocked slots in the progression
  const handleKeyOrScaleChange = (newKey: PitchClassName, newScale: ScaleTypeName) => {
    setSelectedKey(newKey);
    setSelectedScale(newScale);

    setSlots((prev) =>
      prev.map((slot) => {
        if (slot.isLocked) return slot;
        const updatedChord = buildDiatonicChord(newKey, newScale, slot.degree, slot.chord.hasSeventh, 4);
        return {
          ...slot,
          chord: updatedChord,
        };
      }),
    );
  };

  // Audition single current chord
  const handleAuditionCurrentChord = () => {
    const notes = generateVoicingNotes(currentChordDef, voicingStyle, inversion, octaveShift);
    setCurrentVoicedNotes(notes);
    audioSynth.playChord(notes.map((n) => n.frequencyHz), 1.6);
  };

  // Audition specific slot
  const handleAuditionSlot = (index: number) => {
    const slot = slots[index];
    if (!slot) return;

    let notes: NoteInfo[];
    if (enableVoiceLeading && index > 0) {
      const prevNotes = generateVoicingNotes(
        slots[index - 1].chord,
        slots[index - 1].voicingStyle,
        slots[index - 1].inversion,
        slots[index - 1].octaveShift,
      );
      notes = findOptimalVoiceLeading(prevNotes, slot.chord, slot.voicingStyle);
    } else {
      notes = generateVoicingNotes(slot.chord, slot.voicingStyle, slot.inversion, slot.octaveShift);
    }

    setActiveSlotIndex(index);
    setCurrentVoicedNotes(notes);
    audioSynth.playChord(notes.map((n) => n.frequencyHz), 1.5);
  };

  // Toggle slot lock
  const toggleSlotLock = (index: number, e: React.MouseEvent) => {
    e.stopPropagation();
    setSlots((prev) =>
      prev.map((s, i) => (i === index ? { ...s, isLocked: !s.isLocked } : s)),
    );
  };

  // Add new chord slot
  const handleAddSlot = () => {
    if (slots.length >= 8) return;
    const newChord = buildDiatonicChord(selectedKey, selectedScale, selectedDegree, includeSeventh, 4);
    const newSlot: ProgressionSlotState = {
      id: `slot-${Date.now()}`,
      degree: selectedDegree,
      chord: newChord,
      voicingStyle,
      inversion,
      octaveShift,
      isLocked: false,
      durationBeats: 4,
    };
    setSlots([...slots, newSlot]);
  };

  // Remove slot
  const handleRemoveSlot = (index: number, e: React.MouseEvent) => {
    e.stopPropagation();
    if (slots.length <= 1) return;
    setSlots(slots.filter((_, i) => i !== index));
  };

  // Musical Randomizer
  const handleRandomize = () => {
    const randomChoices = getRandomProgression(selectedKey, selectedScale, randomizerStyle, slots.length);
    setSlots((prev) =>
      prev.map((slot, i) => {
        if (slot.isLocked) return slot;
        const choice = randomChoices[i % randomChoices.length];
        const newChord = buildDiatonicChord(selectedKey, selectedScale, choice.degree, choice.hasSeventh, 4);
        return {
          ...slot,
          degree: choice.degree,
          chord: newChord,
        };
      }),
    );
  };

  // Sequencer Playback Loop
  const timerRef = useRef<number | null>(null);
  const currentStepRef = useRef<number>(0);

  const playStep = useCallback((stepIndex: number) => {
    if (slots.length === 0) return;
    const slot = slots[stepIndex % slots.length];
    setActiveSlotIndex(stepIndex % slots.length);

    let notes: NoteInfo[];
    if (enableVoiceLeading && stepIndex > 0) {
      const prevSlot = slots[(stepIndex - 1 + slots.length) % slots.length];
      const prevNotes = generateVoicingNotes(prevSlot.chord, prevSlot.voicingStyle, prevSlot.inversion, prevSlot.octaveShift);
      notes = findOptimalVoiceLeading(prevNotes, slot.chord, slot.voicingStyle);
    } else {
      notes = generateVoicingNotes(slot.chord, slot.voicingStyle, slot.inversion, slot.octaveShift);
    }

    setCurrentVoicedNotes(notes);
    const stepDurationSec = (slot.durationBeats * 60) / bpm;
    audioSynth.playChord(notes.map((n) => n.frequencyHz), stepDurationSec * 0.95);

    timerRef.current = window.setTimeout(() => {
      currentStepRef.current = (stepIndex + 1) % slots.length;
      playStep(currentStepRef.current);
    }, stepDurationSec * 1000);
  }, [slots, bpm, enableVoiceLeading]);

  const togglePlayback = () => {
    if (isPlaying) {
      if (timerRef.current) clearTimeout(timerRef.current);
      audioSynth.stopAll();
      setIsPlaying(false);
      setActiveSlotIndex(-1);
    } else {
      setIsPlaying(true);
      currentStepRef.current = 0;
      playStep(0);
    }
  };

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      if (timerRef.current) clearTimeout(timerRef.current);
      audioSynth.stopAll();
    };
  }, []);

  // Export MIDI file
  const handleExportMidi = () => {
    downloadMidiFile(
      slots,
      bpm,
      enableVoiceLeading,
      `ChordForge_${selectedKey}_${selectedScale.replace(/\s+/g, '')}.mid`,
    );
  };

  // Preset loading helper
  const handleLoadPreset = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const val = e.target.value;
    if (!val) return;

    if (val === 'pop_classic') {
      handleKeyOrScaleChange('C', 'Major');
      setVoicingStyle('Piano');
      setBpm(120);
      const degrees = [1, 5, 6, 4];
      setSlots(degrees.map((d, i) => ({
        id: `pop-${i}`,
        degree: d,
        chord: buildDiatonicChord('C', 'Major', d, false, 4),
        voicingStyle: 'Piano',
        inversion: 0,
        octaveShift: 0,
        isLocked: false,
        durationBeats: 4,
      })));
    } else if (val === 'neo_soul') {
      handleKeyOrScaleChange('F', 'Major');
      setVoicingStyle('Drop 2');
      setBpm(85);
      const degrees = [4, 3, 6, 2];
      setSlots(degrees.map((d, i) => ({
        id: `soul-${i}`,
        degree: d,
        chord: buildDiatonicChord('F', 'Major', d, true, 4),
        voicingStyle: 'Drop 2',
        inversion: 0,
        octaveShift: 0,
        isLocked: false,
        durationBeats: 4,
      })));
    } else if (val === 'jazz_two_five_one') {
      handleKeyOrScaleChange('C', 'Major');
      setVoicingStyle('Drop 2');
      setBpm(110);
      const degrees = [2, 5, 1, 6];
      setSlots(degrees.map((d, i) => ({
        id: `jazz-${i}`,
        degree: d,
        chord: buildDiatonicChord('C', 'Major', d, true, 4),
        voicingStyle: 'Drop 2',
        inversion: 0,
        octaveShift: 0,
        isLocked: false,
        durationBeats: 4,
      })));
    } else if (val === 'lofi_rain') {
      handleKeyOrScaleChange('A', 'Natural Minor');
      setVoicingStyle('Close');
      setBpm(80);
      const degrees = [1, 7, 6, 5];
      setSlots(degrees.map((d, i) => ({
        id: `lofi-${i}`,
        degree: d,
        chord: buildDiatonicChord('A', 'Natural Minor', d, true, 4),
        voicingStyle: 'Close',
        inversion: 0,
        octaveShift: 0,
        isLocked: false,
        durationBeats: 4,
      })));
    }
  };

  return (
    <div className="w-full max-w-4xl mx-auto rounded-xl border border-zinc-800 bg-[#0d0f12] shadow-2xl p-4 md:p-6 text-zinc-200">
      {/* Plugin Header Frame */}
      <div className="flex flex-wrap items-center justify-between gap-3 border-b border-zinc-800/80 pb-4 mb-5">
        <div className="flex items-center gap-3">
          <div className="flex items-center gap-2">
            <span className="w-2.5 h-2.5 rounded-full bg-cyan-400 animate-pulse" />
            <h1 className="text-xl font-bold tracking-wider text-white uppercase font-mono">
              CHORDFORGE
            </h1>
          </div>
          <span className="text-xs text-zinc-500 font-mono">VST3 / Standalone</span>
          <span className="text-xs px-2 py-0.5 rounded bg-zinc-800/60 text-zinc-400 border border-zinc-700/50 font-mono">
            v1.0.0
          </span>
        </div>

        <div className="flex items-center gap-2">
          {/* Preset selector */}
          <div className="relative">
            <select
              defaultValue=""
              onChange={handleLoadPreset}
              className="bg-zinc-900 border border-zinc-700 text-xs text-zinc-300 rounded px-2.5 py-1.5 pr-7 appearance-none cursor-pointer hover:border-zinc-500 focus:outline-none focus:border-cyan-500 font-mono"
            >
              <option value="" disabled>Load Factory Preset...</option>
              <option value="pop_classic">Pop: Four Chords (I - V - vi - IV)</option>
              <option value="neo_soul">R&B / Soul: Neo Soul Velvet</option>
              <option value="jazz_two_five_one">Jazz: Standard ii7 - V7 - Imaj7 - vi7</option>
              <option value="lofi_rain">Lo-Fi: Rainy Window (i - VII - VI - v)</option>
            </select>
            <ChevronDown className="w-3.5 h-3.5 text-zinc-400 absolute right-2 top-2.5 pointer-events-none" />
          </div>

          {/* Mute button */}
          <button
            onClick={() => {
              const next = !isMuted;
              setIsMuted(next);
              audioSynth.setMuted(next);
            }}
            title={isMuted ? 'Unmute Synth' : 'Mute Synth'}
            className="p-1.5 rounded bg-zinc-900 border border-zinc-800 hover:border-zinc-700 text-zinc-400 hover:text-white cursor-pointer"
          >
            {isMuted ? <VolumeX className="w-4 h-4 text-rose-400" /> : <Volume2 className="w-4 h-4" />}
          </button>

          {/* DAW Guide trigger */}
          <button
            onClick={onOpenDawGuide}
            className="px-2.5 py-1.5 rounded text-xs bg-zinc-900 border border-zinc-700 hover:border-cyan-500 text-cyan-400 font-mono cursor-pointer transition-colors"
          >
            DAW Routing
          </button>
        </div>
      </div>

      {/* Control Matrix Row */}
      <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-6 gap-3 mb-5">
        {/* Key */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <label className="block text-[10px] font-bold text-zinc-400 mb-1 font-mono uppercase tracking-wider">
            KEY
          </label>
          <select
            value={selectedKey}
            onChange={(e) => handleKeyOrScaleChange(e.target.value as PitchClassName, selectedScale)}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            {PITCH_CLASSES.map((k) => (
              <option key={k} value={k}>{k}</option>
            ))}
          </select>
        </div>

        {/* Scale */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <label className="block text-[10px] font-bold text-zinc-400 mb-1 font-mono uppercase tracking-wider">
            SCALE
          </label>
          <select
            value={selectedScale}
            onChange={(e) => handleKeyOrScaleChange(selectedKey, e.target.value as ScaleTypeName)}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            {Object.keys(SCALE_INTERVALS).map((s) => (
              <option key={s} value={s}>{s}</option>
            ))}
          </select>
        </div>

        {/* Diatonic Degree & 7th */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <div className="flex items-center justify-between mb-1">
            <label className="text-[10px] font-bold text-zinc-400 font-mono uppercase tracking-wider">
              DEGREE
            </label>
            <label className="flex items-center gap-1 text-[10px] text-zinc-400 cursor-pointer font-mono">
              <input
                type="checkbox"
                checked={includeSeventh}
                onChange={(e) => setIncludeSeventh(e.target.checked)}
                className="accent-cyan-500"
              />
              +7th
            </label>
          </div>
          <select
            value={selectedDegree}
            onChange={(e) => setSelectedDegree(Number(e.target.value))}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            {[1, 2, 3, 4, 5, 6, 7].map((deg) => (
              <option key={deg} value={deg}>
                Degree {deg} ({buildDiatonicChord(selectedKey, selectedScale, deg, includeSeventh).romanNumeral})
              </option>
            ))}
          </select>
        </div>

        {/* Voicing Style */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <label className="block text-[10px] font-bold text-zinc-400 mb-1 font-mono uppercase tracking-wider">
            VOICING
          </label>
          <select
            value={voicingStyle}
            onChange={(e) => setVoicingStyle(e.target.value as VoicingStyleName)}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            {VOICING_STYLES.map((v) => (
              <option key={v} value={v}>{v}</option>
            ))}
          </select>
        </div>

        {/* Inversion */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <label className="block text-[10px] font-bold text-zinc-400 mb-1 font-mono uppercase tracking-wider">
            INVERSION
          </label>
          <select
            value={inversion}
            onChange={(e) => setInversion(Number(e.target.value))}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            <option value={0}>Root Pos</option>
            <option value={1}>1st Inv</option>
            <option value={2}>2nd Inv</option>
            {includeSeventh && <option value={3}>3rd Inv</option>}
          </select>
        </div>

        {/* Octave Shift */}
        <div className="bg-zinc-900/80 p-2.5 rounded border border-zinc-800">
          <label className="block text-[10px] font-bold text-zinc-400 mb-1 font-mono uppercase tracking-wider">
            OCTAVE
          </label>
          <select
            value={octaveShift}
            onChange={(e) => setOctaveShift(Number(e.target.value))}
            className="w-full bg-zinc-950 border border-zinc-700 text-sm font-mono text-white rounded px-2 py-1 cursor-pointer focus:border-cyan-500"
          >
            <option value={-2}>-2 Oct</option>
            <option value={-1}>-1 Oct</option>
            <option value={0}>0 (C4)</option>
            <option value={1}>+1 Oct</option>
            <option value={2}>+2 Oct</option>
          </select>
        </div>
      </div>

      {/* Active Chord Readout Box */}
      <div className="bg-gradient-to-r from-zinc-900/90 to-zinc-950 border border-zinc-800 rounded-lg p-3.5 mb-5 flex flex-wrap items-center justify-between gap-4">
        <div>
          <div className="flex items-center gap-2 mb-1">
            <span className="text-xl font-mono font-bold text-cyan-400">
              {currentChordDef.romanNumeral}
            </span>
            <span className="text-sm font-semibold text-zinc-200">
              {currentChordDef.root} {currentChordDef.qualityName}
            </span>
          </div>
          <div className="text-xs text-zinc-400 font-mono flex items-center gap-1.5 flex-wrap">
            <span>Voiced Notes:</span>
            {currentVoicedNotes.map((n) => (
              <span key={n.midi} className="text-zinc-200 font-bold bg-zinc-800/80 px-1 rounded">
                {n.name}
              </span>
            ))}
            <span>·</span>
            <span>Style: {voicingStyle}</span>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={handleAuditionCurrentChord}
            className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-zinc-800 hover:bg-zinc-700 text-white font-mono text-xs border border-zinc-700 transition cursor-pointer"
          >
            <Play className="w-3.5 h-3.5 fill-current" />
            Audition
          </button>
          <button
            onClick={handleAddSlot}
            className="flex items-center gap-1 px-3 py-1.5 rounded bg-cyan-600 hover:bg-cyan-500 text-zinc-950 font-bold font-mono text-xs transition cursor-pointer"
          >
            <Plus className="w-3.5 h-3.5" />
            Add to Slots
          </button>
        </div>
      </div>

      {/* Progression Builder Section */}
      <div className="bg-zinc-950/70 border border-zinc-800 rounded-lg p-4 mb-5">
        <div className="flex flex-wrap items-center justify-between gap-2 mb-3">
          <div className="flex items-center gap-2">
            <SlidersHorizontal className="w-4 h-4 text-zinc-400" />
            <h3 className="text-xs font-bold font-mono uppercase tracking-wider text-zinc-300">
              Progression Builder ({slots.length} Chords)
            </h3>
          </div>

          <div className="flex items-center gap-3 text-xs font-mono">
            {/* Voice Leading Toggle */}
            <label className="flex items-center gap-1.5 text-zinc-400 cursor-pointer">
              <input
                type="checkbox"
                checked={enableVoiceLeading}
                onChange={(e) => setEnableVoiceLeading(e.target.checked)}
                className="accent-cyan-500"
              />
              Smooth Voice Leading
            </label>

            {/* Tempo */}
            <div className="flex items-center gap-1.5 text-zinc-400">
              <span>BPM:</span>
              <input
                type="number"
                min="40"
                max="260"
                value={bpm}
                onChange={(e) => setBpm(Math.max(40, Math.min(260, Number(e.target.value))))}
                className="w-14 bg-zinc-900 border border-zinc-700 rounded px-1.5 py-0.5 text-white text-center focus:border-cyan-500"
              />
            </div>
          </div>
        </div>

        {/* Slot Buttons Grid */}
        <div className="grid grid-cols-2 sm:grid-cols-4 md:grid-cols-8 gap-2 mb-4">
          {slots.map((slot, idx) => {
            const isCurrentlyPlaying = activeSlotIndex === idx;
            return (
              <div
                key={slot.id}
                onClick={() => handleAuditionSlot(idx)}
                className={`relative rounded-md p-2.5 border transition-all cursor-pointer select-none flex flex-col justify-between h-24 ${
                  isCurrentlyPlaying
                    ? 'bg-cyan-950/90 border-cyan-400 shadow-md ring-1 ring-cyan-400'
                    : 'bg-zinc-900/90 hover:bg-zinc-800/90 border-zinc-800 hover:border-zinc-700'
                }`}
              >
                {/* Header: Roman & lock */}
                <div className="flex items-center justify-between">
                  <span className="text-xs font-mono font-bold text-cyan-400">
                    {slot.chord.romanNumeral}
                  </span>
                  <div className="flex items-center gap-1">
                    <button
                      onClick={(e) => toggleSlotLock(idx, e)}
                      title={slot.isLocked ? 'Slot Locked' : 'Slot Unlocked'}
                      className="text-zinc-500 hover:text-white"
                    >
                      {slot.isLocked ? (
                        <Lock className="w-3 h-3 text-amber-400" />
                      ) : (
                        <Unlock className="w-3 h-3 text-zinc-600" />
                      )}
                    </button>
                    {slots.length > 1 && (
                      <button
                        onClick={(e) => handleRemoveSlot(idx, e)}
                        title="Remove chord"
                        className="text-zinc-600 hover:text-rose-400"
                      >
                        <Trash2 className="w-3 h-3" />
                      </button>
                    )}
                  </div>
                </div>

                {/* Chord Name */}
                <div className="my-1">
                  <div className="text-xs font-semibold text-zinc-200 truncate">
                    {slot.chord.root} {slot.chord.qualityName}
                  </div>
                  <div className="text-[10px] text-zinc-500 font-mono truncate">
                    {slot.voicingStyle}
                  </div>
                </div>

                {/* Duration indicator */}
                <div className="flex items-center justify-between text-[9px] text-zinc-500 font-mono">
                  <span>Slot {idx + 1}</span>
                  <span>1 Bar</span>
                </div>
              </div>
            );
          })}
        </div>

        {/* Progression Action Bar */}
        <div className="flex flex-wrap items-center justify-between gap-3 pt-2 border-t border-zinc-800/60">
          <div className="flex items-center gap-2">
            {/* Play/Stop Sequencer */}
            <button
              onClick={togglePlayback}
              className={`flex items-center gap-2 px-4 py-2 rounded font-mono text-xs font-bold cursor-pointer transition ${
                isPlaying
                  ? 'bg-amber-500 hover:bg-amber-400 text-zinc-950'
                  : 'bg-emerald-600 hover:bg-emerald-500 text-white'
              }`}
            >
              {isPlaying ? (
                <>
                  <Square className="w-3.5 h-3.5 fill-current" />
                  Stop Playback
                </>
              ) : (
                <>
                  <Play className="w-3.5 h-3.5 fill-current" />
                  Play Progression
                </>
              )}
            </button>

            {/* Randomize with Style */}
            <div className="flex items-center rounded bg-zinc-900 border border-zinc-700 overflow-hidden">
              <button
                onClick={handleRandomize}
                className="flex items-center gap-1.5 px-3 py-2 text-xs font-mono text-zinc-200 hover:bg-zinc-800 cursor-pointer"
                title="Generate new progression respecting locked slots"
              >
                <Shuffle className="w-3.5 h-3.5 text-cyan-400" />
                Randomize
              </button>
              <select
                value={randomizerStyle}
                onChange={(e) => setRandomizerStyle(e.target.value as RandomizerStyleName)}
                className="bg-zinc-950 border-l border-zinc-700 text-[11px] font-mono text-zinc-400 px-2 py-2 cursor-pointer focus:outline-none"
              >
                <option value="Pop">Pop</option>
                <option value="Jazz">Jazz</option>
                <option value="Minor/Sad">Minor/Sad</option>
                <option value="R&B / Soul">R&B</option>
                <option value="EDM Modern">EDM</option>
              </select>
            </div>
          </div>

          {/* Export Standard MIDI File */}
          <button
            onClick={handleExportMidi}
            className="flex items-center gap-1.5 px-4 py-2 rounded bg-zinc-900 border border-cyan-800/80 hover:border-cyan-500 text-cyan-400 font-mono text-xs font-bold transition cursor-pointer"
            title="Download true Standard MIDI File (.mid) for Ableton or FL Studio"
          >
            <Download className="w-3.5 h-3.5" />
            Export MIDI (.mid)
          </button>
        </div>
      </div>

      {/* Real-time Piano Visualizer */}
      <PianoVisualizer activeNotes={currentVoicedNotes} startOctave={2} octaveCount={3} />
    </div>
  );
};
