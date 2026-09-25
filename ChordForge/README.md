# ChordForge (VST3 / Standalone)

> **Build chords. Shape progressions. Make music.**

ChordForge is a professional Windows x64 VST3 MIDI chord-generation audio plugin and instrument utility built with modern **C++20** and the **JUCE** framework. Rather than processing audio, ChordForge generates high-fidelity MIDI notes and chord progressions in real-time for feeding downstream virtual instruments, software synthesizers, and piano samplers.

---

## Features

- **Algorithmic Music Theory Engine**:
  - Full chromatic pitch representation (0–127 MIDI numbers, frequencies, enharmonics).
  - 9 Scale Types: Major (Ionian), Natural Minor (Aeolian), Dorian, Phrygian, Lydian, Mixolydian, Locrian, Harmonic Minor, Melodic Minor.
  - Diatonic scale degree chord solver: stacking thirds algorithmically (triads, 7ths, 9ths, 11ths, 13ths, sus2, sus4).
  - Roman numeral analysis (I, ii, iii, IV, V, vi, vii°, Imaj7, ii7, etc.).

- **Advanced Voicing & Inversion Engine**:
  - 8 Voicing Styles: **Close**, **Open**, **Wide**, **Drop 2**, **Piano**, **Guitar**, **Pad**, **Bass + Chord**.
  - Inversions 0 (Root), 1 (1st), 2 (2nd), 3 (3rd).
  - Octave shifting with bounded MIDI range clamping.

- **Voice Leading Optimization**:
  - Minimum voice movement algorithm evaluating pitch distance, maximum jump, range constraints, and spacing penalties between consecutive chords.

- **Progression Builder**:
  - Multi-slot chord sequencer with individual slot locking (`[I locked] [V] [vi] [IV locked]`).
  - Musical Randomizer with style presets (Pop, Jazz, Minor/Sad, R&B/Soul, EDM, Ambient).
  - DAW Playhead / Transport synchronization with PPQ position tracking.

- **MIDI Engine & Trigger Modes**:
  - Real-time lock-free thread-safe MIDI generation (`juce::MidiBuffer`).
  - 4 Modes: DAW Host Sequencer, Root Trigger, Scale Degree Trigger, Progression Slot Trigger.
  - Standard MIDI File (.mid) exporter with 960 PPQ resolution for immediate DAW drag-and-drop.

- **Assembly & SIMD Ready**:
  - Clean abstraction layer (`ChordForge::Optimized`) with C++ reference baseline and benchmarking tools.

---

## Directory Architecture

```text
ChordForge/
├── CMakeLists.txt              # Root CMake build configuration
├── README.md                   # Plugin documentation & build guide
├── LICENSE                     # MIT License
├── AI_HANDOFF.md               # Local AI CLI handoff specification
├── DAW_TESTING.md              # Ableton Live & FL Studio manual testing manual
│
├── Source/
│   ├── Plugin/
│   │   ├── PluginProcessor.h   # juce::AudioProcessor, APVTS, real-time MIDI buffer
│   │   ├── PluginProcessor.cpp # Transport sync & thread-safe message queues
│   │   ├── PluginEditor.h      # Hardware-inspired custom dark UI
│   │   └── PluginEditor.cpp    # Piano visualizer, slot UI, look-and-feel
│   │
│   ├── Music/
│   │   ├── Note.h / .cpp       # MIDI pitch class & frequency conversion
│   │   ├── Scale.h / .cpp      # 9 scale models & interval bitmasks
│   │   ├── Chord.h / .cpp      # Diatonic degree solver & Roman numeral parser
│   │   ├── Voicing.h / .cpp    # Drop-2, open, wide, piano, pad voicing algorithms
│   │   ├── VoiceLeading.h/.cpp # Pitch distance & voice crossing minimizer
│   │   └── Progression.h/.cpp  # Slot progression builder & randomizer
│   │
│   ├── MIDI/
│   │   ├── MidiGenerator.h/.cpp# Sample-accurate note-on/off sequencer
│   │   ├── MidiProcessor.h/.cpp# MIDI input trigger modes
│   │   └── MidiExporter.h/.cpp # Standard MIDI file (.mid) binary writer
│   │
│   ├── Assembly/
│   │   └── OptimizedMath.h/.cpp# SIMD/Assembly abstraction & microbenchmarks
│   │
│   ├── Presets/
│   │   └── PresetManager.h/.cpp# Factory presets (Pop, Jazz, Lo-Fi, Neo-Soul)
│   │
│   └── Tests/
│       └── MusicTheoryTests.cpp# Automated test suite
│
└── tests/
    └── CMakeLists.txt          # Test runner executable configuration
```

---

## Windows Prerequisites

To compile ChordForge on Windows:

1. **Operating System**: Windows 10 or Windows 11 (64-bit).
2. **Visual Studio**: Visual Studio 2022 (Community, Professional, or Enterprise) with the **"Desktop development with C++"** workload selected.
3. **CMake**: Version 3.22 or newer (included in Visual Studio or from [cmake.org](https://cmake.org)).
4. **Git**: Installed and available in PATH.

---

## How to Configure & Build on Windows

Open **Developer Command Prompt for VS 2022** or **PowerShell**:

```bash
# 1. Clone repository
git clone https://github.com/your-org/ChordForge.git
cd ChordForge

# 2. Configure CMake with Visual Studio 2022 generator (fetches JUCE 7 automatically)
cmake -B build -G "Visual Studio 17 2022" -A x64

# 3. Build Release VST3 and Standalone
cmake --build build --config Release --target ChordForge_VST3 ChordForge_Standalone

# 4. Build and Run Unit Tests
cmake --build build --config Release --target ChordForgeTests
./build/tests/Release/ChordForgeTests.exe
```

---

## Where the VST3 is Produced

When built with `COPY_PLUGIN_AFTER_BUILD TRUE`, JUCE automatically copies the VST3 bundle to your system VST3 directory:

- **Windows Standard VST3 Location**:
  `C:\Program Files\Common Files\VST3\ChordForge.vst3`
- **Build Output Directory**:
  `build/ChordForge_artefacts/Release/VST3/ChordForge.vst3`

---

## Automated Unit Tests

Run the test suite to verify music theory compliance (C Major diatonic degrees I..vii°, inversions, 7th chords, voice leading):

```bash
./build/tests/Release/ChordForgeTests.exe
```

Expected output:
```text
===========================================
CHORDFORGE MUSIC THEORY & MIDI TEST SUITE
===========================================
Running testNotesAndPitchClasses...
Running testScales...
Running testCMajorDiatonicChords...
Running testInversions...
Running testSeventhChords...
Running testVoiceLeading...
Running testOptimizedMath...

-------------------------------------------
TOTAL PASSED: 38
TOTAL FAILED: 0
-------------------------------------------
```

---

## Roadmap

- [ ] Native drag-and-drop MIDI directly out of the plugin window.
- [ ] AVX2 SIMD / NASM x86-64 assembly kernel for large-batch pitch transformations.
- [ ] Arpeggiator and rhythm strumming engine.
- [ ] Modal interchange and borrowed chord suggestions.
- [ ] macOS AU / CLAP target support.
