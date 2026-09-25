#include "OptimizedMath.h"
#include <algorithm>
#include <chrono>

namespace ChordForge::Optimized
{

void batchTransposeNotes(const int* input, int* output, size_t count, int semitoneShift) noexcept
{
    if (input == nullptr || output == nullptr) return;

    // Portable C++ implementation with compiler vectorization hints.
    // In future x86-64 assembly / AVX2 implementation:
    // _mm256_loadu_si256, _mm256_add_epi32, _mm256_max_epi32, _mm256_min_epi32
    for (size_t i = 0; i < count; ++i)
    {
        int val = input[i] + semitoneShift;
        output[i] = std::clamp(val, 0, 127);
    }
}

void batchClampMidi(const int* input, int* output, size_t count) noexcept
{
    if (input == nullptr || output == nullptr) return;

    for (size_t i = 0; i < count; ++i)
    {
        output[i] = std::clamp(input[i], 0, 127);
    }
}

uint16_t computePitchClassMask(const int* notes, size_t count) noexcept
{
    if (notes == nullptr || count == 0) return 0;

    uint16_t mask = 0;
    for (size_t i = 0; i < count; ++i)
    {
        int pc = (notes[i] % 12 + 12) % 12;
        mask |= static_cast<uint16_t>(1 << pc);
    }
    return mask;
}

bool checkNotesInScaleMask(uint16_t scaleMask, const int* notes, size_t count) noexcept
{
    if (notes == nullptr || count == 0) return true;

    for (size_t i = 0; i < count; ++i)
    {
        int pc = (notes[i] % 12 + 12) % 12;
        if ((scaleMask & (1 << pc)) == 0)
        {
            return false;
        }
    }
    return true;
}

BenchmarkResult runTransposeBenchmark(size_t iterations)
{
    constexpr size_t TEST_SIZE = 128;
    int inputNotes[TEST_SIZE];
    int outputNotes[TEST_SIZE];

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        inputNotes[i] = static_cast<int>(30 + (i % 70));
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t iter = 0; iter < iterations; ++iter)
    {
        batchTransposeNotes(inputNotes, outputNotes, TEST_SIZE, 7);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::micro> elapsed = end - start;
    double opsPerSec = (static_cast<double>(iterations * TEST_SIZE) / (elapsed.count() / 1000000.0));

    BenchmarkResult res;
    res.algorithmName = "Batch Transpose C++ Baseline";
    res.iterations = iterations;
    res.elapsedMicroseconds = elapsed.count();
    res.throughputOpsPerSec = opsPerSec;

    return res;
}

} // namespace ChordForge::Optimized
