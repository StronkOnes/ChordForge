import React from 'react';
import { NoteInfo } from '../types/music';
import { audioSynth } from '../engine/audioSynth';

interface PianoVisualizerProps {
  activeNotes: NoteInfo[];
  startOctave?: number;
  octaveCount?: number;
}

export const PianoVisualizer: React.FC<PianoVisualizerProps> = ({
  activeNotes,
  startOctave = 3,
  octaveCount = 3,
}) => {
  const activeMidis = new Set(activeNotes.map((n) => n.midi));

  // Build key array from startOctave to startOctave + octaveCount
  const startMidi = (startOctave + 1) * 12; // C3 = 48
  const totalNotes = octaveCount * 12;

  const whiteKeys: { midi: number; name: string; pc: number; freq: number }[] = [];
  const blackKeys: { midi: number; name: string; pc: number; freq: number; whiteIndex: number }[] = [];

  const PC_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
  const IS_BLACK = [false, true, false, true, false, false, true, false, true, false, true, false];

  let whiteCounter = 0;
  for (let m = startMidi; m < startMidi + totalNotes; m++) {
    const pc = m % 12;
    const oct = Math.floor(m / 12) - 1;
    const name = `${PC_NAMES[pc]}${oct}`;
    const freq = 440 * Math.pow(2, (m - 69) / 12);

    if (!IS_BLACK[pc]) {
      whiteKeys.push({ midi: m, name, pc, freq });
      whiteCounter++;
    } else {
      blackKeys.push({ midi: m, name, pc, freq, whiteIndex: whiteCounter });
    }
  }

  const handleKeyClick = (freq: number) => {
    audioSynth.playChord([freq], 0.8);
  };

  const totalWhite = whiteKeys.length;

  return (
    <div className="w-full bg-zinc-950 p-3 rounded-lg border border-zinc-800">
      <div className="flex items-center justify-between mb-2">
        <div className="flex items-center gap-2 text-xs font-mono text-zinc-400">
          <span className="text-zinc-500 uppercase tracking-wider text-[11px] font-semibold">Interactive Piano Visualizer</span>
          <span>·</span>
          <span>{activeNotes.length} active voice{activeNotes.length === 1 ? '' : 's'}</span>
        </div>
        <div className="flex items-center gap-2 text-xs font-mono text-cyan-400">
          {activeNotes.map((n) => (
            <span key={n.midi} className="bg-cyan-950/70 border border-cyan-800/80 px-1.5 py-0.5 rounded text-[11px]">
              {n.name} <span className="text-cyan-600 font-sans">({n.midi})</span>
            </span>
          ))}
        </div>
      </div>

      <div className="relative h-28 w-full select-none overflow-hidden rounded border border-zinc-800 bg-zinc-900">
        {/* White Keys */}
        <div className="flex w-full h-full">
          {whiteKeys.map((k) => {
            const isActive = activeMidis.has(k.midi);
            return (
              <button
                key={k.midi}
                onClick={() => handleKeyClick(k.freq)}
                title={`${k.name} (MIDI ${k.midi}) - ${k.freq.toFixed(1)} Hz`}
                className={`flex-1 h-full border-r border-zinc-300/80 last:border-r-0 transition-colors relative flex flex-col justify-end items-center pb-1 text-[9px] font-mono cursor-pointer ${
                  isActive
                    ? 'bg-cyan-400 text-zinc-950 font-bold shadow-inner'
                    : 'bg-zinc-100 hover:bg-zinc-200 text-zinc-600'
                }`}
              >
                {k.pc === 0 ? <span className="opacity-75">{k.name}</span> : null}
                {isActive ? (
                  <span className="w-2 h-2 rounded-full bg-cyan-900 mb-0.5" />
                ) : null}
              </button>
            );
          })}
        </div>

        {/* Black Keys */}
        {blackKeys.map((k) => {
          const isActive = activeMidis.has(k.midi);
          // Position relative to adjacent white key width
          const leftPercent = ((k.whiteIndex - 0.35) / totalWhite) * 100;
          const widthPercent = (0.7 / totalWhite) * 100;

          return (
            <button
              key={k.midi}
              onClick={() => handleKeyClick(k.freq)}
              title={`${k.name} (MIDI ${k.midi}) - ${k.freq.toFixed(1)} Hz`}
              style={{
                left: `${leftPercent}%`,
                width: `${widthPercent}%`,
                height: '62%',
              }}
              className={`absolute top-0 z-10 rounded-b transition-colors flex flex-col justify-end items-center pb-1 text-[8px] font-mono cursor-pointer ${
                isActive
                  ? 'bg-sky-500 text-zinc-950 font-bold border border-cyan-300 shadow-md'
                  : 'bg-zinc-900 hover:bg-zinc-800 text-zinc-400 border border-zinc-950'
              }`}
            >
              {isActive ? <span className="w-1.5 h-1.5 rounded-full bg-white mb-0.5" /> : null}
            </button>
          );
        })}
      </div>
    </div>
  );
};
