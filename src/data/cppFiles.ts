export interface CppFileItem {
  path: string;
  category: 'CMake' | 'Plugin' | 'Music Engine' | 'MIDI Engine' | 'Assembly' | 'Presets' | 'Tests' | 'Documentation';
  description: string;
}

export const CPP_FILES_CATALOG: CppFileItem[] = [
  {
    path: 'ChordForge/CMakeLists.txt',
    category: 'CMake',
    description: 'Root CMakeLists.txt integrating JUCE 7 via FetchContent, target ChordForge with VST3 & Standalone formats.',
  },
  {
    path: 'ChordForge/tests/CMakeLists.txt',
    category: 'CMake',
    description: 'CMake build script for building the automated C++ unit test runner executable.',
  },
  {
    path: 'ChordForge/Source/Plugin/PluginProcessor.h',
    category: 'Plugin',
    description: 'juce::AudioProcessor declaration with AudioProcessorValueTreeState parameter layout and transport synchronization.',
  },
  {
    path: 'ChordForge/Source/Plugin/PluginProcessor.cpp',
    category: 'Plugin',
    description: 'PluginProcessor implementation with real-time thread safety, APVTS bindings, and XML state restoration.',
  },
  {
    path: 'ChordForge/Source/Plugin/PluginEditor.h',
    category: 'Plugin',
    description: 'juce::AudioProcessorEditor with hardware dark theme LookAndFeel and slot progression GUI.',
  },
  {
    path: 'ChordForge/Source/Plugin/PluginEditor.cpp',
    category: 'Plugin',
    description: 'Hardware UI implementation with custom ComboBox/Button styling, 2-octave piano keyboard, and MIDI exporter hook.',
  },
  {
    path: 'ChordForge/Source/Music/Note.h',
    category: 'Music Engine',
    description: 'MIDI pitch class enum (0..11), MIDI note number (0..127), frequency calculations, enharmonics, and octave transposition.',
  },
  {
    path: 'ChordForge/Source/Music/Note.cpp',
    category: 'Music Engine',
    description: 'Implementation of Note conversions, A4 440Hz tuning, clamping, and pitch class parsing.',
  },
  {
    path: 'ChordForge/Source/Music/Scale.h',
    category: 'Music Engine',
    description: 'Scale models for 9 musical modes (Major, Natural Minor, Dorian, Phrygian, Lydian, Mixolydian, Locrian, Harmonic, Melodic).',
  },
  {
    path: 'ChordForge/Source/Music/Scale.cpp',
    category: 'Music Engine',
    description: 'Scale interval tables, 12-bit scale pitch masks, and scale degree lookup algorithm.',
  },
  {
    path: 'ChordForge/Source/Music/Chord.h',
    category: 'Music Engine',
    description: 'Chord qualities (Triads, 7ths, Extensions) and Roman numeral representation (I, ii, iii, IV, V, vi, vii°).',
  },
  {
    path: 'ChordForge/Source/Music/Chord.cpp',
    category: 'Music Engine',
    description: 'Algorithmic diatonic third stacking by scale degree (1..7) and quality identification.',
  },
  {
    path: 'ChordForge/Source/Music/Voicing.h',
    category: 'Music Engine',
    description: 'Voicing transformations: Close, Open, Wide, Drop-2, Piano, Guitar, Pad, Bass+Chord, and inversions.',
  },
  {
    path: 'ChordForge/Source/Music/Voicing.cpp',
    category: 'Music Engine',
    description: 'Pure functional implementation of Drop-2, open spreads, octave shift, and safety clamping.',
  },
  {
    path: 'ChordForge/Source/Music/VoiceLeading.h',
    category: 'Music Engine',
    description: 'Voice leading cost minimization evaluating total pitch distance, max voice jump, and spacing penalties.',
  },
  {
    path: 'ChordForge/Source/Music/VoiceLeading.cpp',
    category: 'Music Engine',
    description: 'Optimal voicing permutation search algorithm across inversions and octave shifts.',
  },
  {
    path: 'ChordForge/Source/Music/Progression.h',
    category: 'Music Engine',
    description: 'Multi-slot chord progression model with lock toggles, duration, and musical randomizer styles.',
  },
  {
    path: 'ChordForge/Source/Music/Progression.cpp',
    category: 'Music Engine',
    description: 'Progression sequence management and diatonic randomization heuristics.',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiGenerator.h',
    category: 'MIDI Engine',
    description: 'Real-time sample-accurate note-on/note-off generator into juce::MidiBuffer without audio thread allocations.',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiGenerator.cpp',
    category: 'MIDI Engine',
    description: 'DAW playhead PPQ tracking, loop wrapping, and voice slot scheduling.',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiProcessor.h',
    category: 'MIDI Engine',
    description: 'MIDI input trigger modes (Root Trigger, Scale Degree Trigger, Progression Slot Trigger).',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiProcessor.cpp',
    category: 'MIDI Engine',
    description: 'Live MIDI keyboard interception and chord re-triggering logic.',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiExporter.h',
    category: 'MIDI Engine',
    description: 'Standard MIDI File (Type 1) writer producing .mid files for DAW drag-and-drop.',
  },
  {
    path: 'ChordForge/Source/MIDI/MidiExporter.cpp',
    category: 'MIDI Engine',
    description: 'Writing 960 PPQ tracks, tempo meta events, and time-stamped note events.',
  },
  {
    path: 'ChordForge/Source/Assembly/OptimizedMath.h',
    category: 'Assembly',
    description: 'C++ SIMD baseline and clean abstraction for future x86-64 NASM/MASM assembly modules.',
  },
  {
    path: 'ChordForge/Source/Assembly/OptimizedMath.cpp',
    category: 'Assembly',
    description: 'Vectorized note transpositions, pitch class masks, and performance microbenchmarking harness.',
  },
  {
    path: 'ChordForge/Source/Presets/PresetManager.h',
    category: 'Presets',
    description: 'Factory preset manager with JUCE ValueTree serialization across 7 musical genres.',
  },
  {
    path: 'ChordForge/Source/Presets/PresetManager.cpp',
    category: 'Presets',
    description: 'Factory presets for Pop, Hip Hop, R&B, Jazz, Lo-Fi, Ambient, and House.',
  },
  {
    path: 'ChordForge/Source/Tests/MusicTheoryTests.cpp',
    category: 'Tests',
    description: 'Automated test suite verifying C Major diatonic triads, inversions, 7th chords, and voice leading.',
  },
  {
    path: 'ChordForge/README.md',
    category: 'Documentation',
    description: 'Complete build, configuration, and architecture manual for Windows Visual Studio 2022.',
  },
  {
    path: 'ChordForge/DAW_TESTING.md',
    category: 'Documentation',
    description: 'Step-by-step Ableton Live & FL Studio MIDI routing and verification manual.',
  },
  {
    path: 'ChordForge/AI_HANDOFF.md',
    category: 'Documentation',
    description: 'Detailed specification for local AI coding CLI to complete, build, and optimize on Windows.',
  },
];
