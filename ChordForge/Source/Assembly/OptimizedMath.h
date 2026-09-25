#pragma once

#include <cstdint>
#include <cstddef>

namespace ChordForge::Optimized
{

// Function signatures matching both C++ baseline and future x86-64 NASM/MASM SIMD implementations

/**
 * Transpose a batch of MIDI notes by a fixed semitone shift, with clamping [0..127].
 */
void batchTransposeNotes(const int* input, int* output, size_t count, int semitoneShift) noexcept;

/**
 * Clamp a buffer of arbitrary integers into valid MIDI note numbers [0..127].
 */
void batchClampMidi(const int* input, int* output, size_t count) noexcept;

/**
 * Calculate 12-bit pitch class mask for an array of MIDI notes.
 */
uint16_t computePitchClassMask(const int* notes, size_t count) noexcept;

/**
 * Verify whether all notes in the array are contained within the given 12-bit scale mask.
 */
bool checkNotesInScaleMask(uint16_t scaleMask, const int* notes, size_t count) noexcept;

/**
 * Benchmark runner for profiling performance of optimized math vs standard loops.
 */
struct BenchmarkResult
{
    const char* algorithmName;
    size_t iterations;
    double elapsedMicroseconds;
    double throughputOpsPerSec;
};

BenchmarkResult runTransposeBenchmark(size_t iterations = 100000);

} // namespace ChordForge::Optimized
