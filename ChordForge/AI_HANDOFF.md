# ChordForge AI Handoff Specification

## 1. Project Context & Purpose

This repository contains the complete first production-grade implementation of **ChordForge**, a Windows x64 VST3 MIDI chord generator and progression utility built in modern **C++20** with **JUCE 7.0.9** and **CMake 3.22+**.

This document is prepared specifically for an AI coding CLI (or software engineer) operating locally on Windows with Visual Studio 2022 to inspect, compile, debug, complete, optimize, and polish the plugin.

---

## 2. Implementation Status Summary

| Subsystem | Status | Key Files | Notes |
| :--- | :--- | :--- | :--- |
| **CMake Build System** | **Complete** | `CMakeLists.txt`, `tests/CMakeLists.txt` | Auto-fetches JUCE 7 via `FetchContent`; configures VST3, Standalone, and Unit Test targets. |
| **Music Theory Engine** | **Complete** | `Source/Music/Note.*`, `Scale.*`, `Chord.*` | Implements 9 scales, 20 chord qualities, chromatic note mapping, diatonic third-stacking solver, Roman numerals. |
| **Voicing Engine** | **Complete** | `Source/Music/Voicing.*` | 8 Voicing styles (Close, Open, Wide, Drop-2, Piano, Guitar, Pad, Bass+Chord) & 4 inversions. |
| **Voice Leading** | **Complete** | `Source/Music/VoiceLeading.*` | Search algorithm evaluating total pitch distance, maximum voice jump, range boundaries, and spacing. |
| **Progression Builder** | **Complete** | `Source/Music/Progression.*` | Slot-based sequencer, lockable slots, musical randomizer heuristics for Pop, Jazz, R&B, Lo-Fi, EDM. |
| **MIDI Engine** | **Complete** | `Source/MIDI/MidiGenerator.*`, `MidiProcessor.*` | Real-time thread-safe `juce::MidiBuffer` processor; 4 trigger modes; DAW playhead PPQ tracking. |
| **MIDI Exporter** | **Complete** | `Source/MIDI/MidiExporter.*` | Standard MIDI File (Type 1) writer (960 PPQ, tempo meta, note on/off events). |
| **Preset Architecture** | **Complete** | `Source/Presets/PresetManager.*` | JUCE ValueTree XML serialization with factory presets across 7 genres. |
| **Plugin Processor** | **Complete** | `Source/Plugin/PluginProcessor.*` | `juce::AudioProcessor` with APVTS parameter layout, transport sync, thread-safe UI auditioning queues. |
| **Plugin Editor UI** | **Complete** | `Source/Plugin/PluginEditor.*` | Hardware-inspired dark UI with custom LookAndFeel, slot buttons, keyboard visualizer, and parameter bindings. |
| **Assembly Abstraction** | **Complete** | `Source/Assembly/OptimizedMath.*` | Portable C++ SIMD baseline with benchmarking harness ready for NASM x86-64 kernels. |
| **Unit Test Suite** | **Complete** | `Source/Tests/MusicTheoryTests.cpp` | Comprehensive automated tests covering all Section 30 requirements. |

---

## 3. Completed Features

1. **Music Theory Core**: Full MIDI note number calculations [0..127], frequency in Hz, enharmonic spellings, interval tables for 9 musical modes, algorithmic chord construction by scale degree.
2. **Harmonic Analysis**: Roman numeral generation for major and minor scales (`I`, `ii`, `iii`, `IV`, `V`, `vi`, `vii°`, `Imaj7`, `ii7`, `V7`, `viiø7`, etc.).
3. **Voicing Transformations**: Pure functional transformations for close, drop-2, open, wide, piano, pad, guitar, and bass+chord.
4. **Voice Leading Optimizer**: Algorithmic cost evaluation minimizing motion between subsequent chords while preventing voice crossing.
5. **Real-Time Safe Audio Thread**: No heap allocations (`malloc`, `new`, `std::vector::reserve`) inside `processBlock()`.
6. **Transport Sync**: Hooks into host playhead to synchronize chord timing with host DAW BPM and PPQ position.
7. **Trigger Modes**: Sequencer, Root Trigger, Scale Degree Trigger, and Progression Slot Trigger.
8. **File Export**: Direct generation of valid Standard MIDI (.mid) binary files.

---

## 4. Pending / Next Recommended Engineering Tasks

1. **Native Drag-and-Drop MIDI**:
   - Currently, MIDI export is triggered via the "Export MIDI (.mid)" button which opens a file dialog.
   - Upgrade this to a direct mouse-drag interaction: implement `juce::DragAndDropContainer` in `PluginEditor` so users can click and drag a MIDI icon directly into Ableton's or FL Studio's arrangement track.
2. **x86-64 NASM Assembly Kernel**:
   - In `Source/Assembly/OptimizedMath.cpp`, the current operations are vectorized by modern MSVC C++ compiler flags (`/O2 /arch:AVX2`).
   - If profiling reveals bottlenecks during large real-time batch queries (e.g. polyphonic voice leading search trees), write NASM/MASM assembly modules in `Source/Assembly/x64/` matching the signatures in `OptimizedMath.h`.
3. **Arpeggiator Engine**:
   - Extend `MidiGenerator` to provide arpeggio patterns (Up, Down, Up-Down, Random, Chord Strum) with subdivision rates (1/8, 1/16, 1/16T).
4. **Enhanced Keyboard Focus**:
   - Enable computer keyboard shortcut hotkeys (1-8 to trigger progression slots, spacebar to audition).

---

## 5. Build & Validation Instructions on Windows

### Command Line (Developer PowerShell or Command Prompt for VS 2022)

```cmd
:: 1. Navigate to ChordForge folder
cd ChordForge

:: 2. Generate CMake build tree (Visual Studio 2022 x64)
cmake -B build -G "Visual Studio 17 2022" -A x64

:: 3. Build Release Plugin and Standalone
cmake --build build --config Release --target ChordForge_VST3 ChordForge_Standalone

:: 4. Build and Run Unit Tests
cmake --build build --config Release --target ChordForgeTests
build\tests\Release\ChordForgeTests.exe
```

### Visual Studio IDE
1. Open Visual Studio 2022.
2. Select **Open a local folder** and choose the `ChordForge` directory.
3. Visual Studio will automatically detect `CMakeLists.txt` and configure the targets.
4. Select `ChordForge_Standalone.exe` or `ChordForgeTests.exe` in the target dropdown and hit **F5** to build and run with debugging.

---

## 6. Known Considerations
- On some hosts (e.g. Ableton Live), VST3 MIDI effects require the track's **MIDI From** and **Monitor: In** to be routed to an instrument track as detailed in `DAW_TESTING.md`.
- CMake automatically downloads JUCE 7.0.9 during the initial configuration via `FetchContent`. Ensure an internet connection is available during first CMake configure, or set `-DJUCE_DIR=...` to use a local JUCE clone.
