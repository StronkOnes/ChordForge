import React, { useState, useEffect } from 'react';
import { runBrowserMusicTheoryTests, TestResult } from '../engine/testRunner';
import { CheckCircle2, XCircle, Play, RefreshCw, Terminal } from 'lucide-react';

export const TestConsole: React.FC = () => {
  const [testResults, setTestResults] = useState<TestResult[]>([]);
  const [passedCount, setPassedCount] = useState<number>(0);
  const [failedCount, setFailedCount] = useState<number>(0);
  const [hasRun, setHasRun] = useState<boolean>(false);

  const executeTests = () => {
    const { results, passedCount: passed, failedCount: failed } = runBrowserMusicTheoryTests();
    setTestResults(results);
    setPassedCount(passed);
    setFailedCount(failed);
    setHasRun(true);
  };

  useEffect(() => {
    executeTests();
  }, []);

  const total = testResults.length;

  return (
    <div className="w-full max-w-4xl mx-auto rounded-xl border border-zinc-800 bg-[#0d0f12] p-5 shadow-2xl font-mono text-zinc-300">
      {/* Header */}
      <div className="flex flex-wrap items-center justify-between gap-3 border-b border-zinc-800 pb-4 mb-4">
        <div>
          <h2 className="text-lg font-bold text-white flex items-center gap-2">
            <Terminal className="w-5 h-5 text-cyan-400" />
            Music Theory & Engine Verification Suite
          </h2>
          <p className="text-xs text-zinc-400 mt-0.5">
            Section 30 specification tests: C Major Diatonic Triads I..vii°, Inversions, 7ths, Voice Leading, MIDI bytes.
          </p>
        </div>

        <button
          onClick={executeTests}
          className="flex items-center gap-2 px-3 py-1.5 rounded bg-zinc-900 border border-zinc-700 hover:border-cyan-500 text-xs text-cyan-400 font-bold transition cursor-pointer"
        >
          <RefreshCw className="w-3.5 h-3.5" />
          Rerun All Tests
        </button>
      </div>

      {/* Summary Scoreboard */}
      <div className="grid grid-cols-3 gap-3 mb-5 text-center">
        <div className="bg-zinc-900/80 border border-zinc-800 p-3 rounded-lg">
          <div className="text-xs text-zinc-400 uppercase">Total Tests</div>
          <div className="text-2xl font-bold text-white mt-1">{total}</div>
        </div>
        <div className="bg-emerald-950/40 border border-emerald-800/80 p-3 rounded-lg">
          <div className="text-xs text-emerald-400 uppercase">Passed</div>
          <div className="text-2xl font-bold text-emerald-400 mt-1">{passedCount}</div>
        </div>
        <div className="bg-rose-950/40 border border-rose-800/80 p-3 rounded-lg">
          <div className="text-xs text-rose-400 uppercase">Failed</div>
          <div className="text-2xl font-bold text-rose-400 mt-1">{failedCount}</div>
        </div>
      </div>

      {/* Test Items Table */}
      <div className="border border-zinc-800 rounded-lg overflow-hidden bg-zinc-950/70 max-h-[480px] overflow-y-auto">
        <table className="w-full text-xs text-left border-collapse">
          <thead className="bg-zinc-900 text-zinc-400 text-[10px] uppercase tracking-wider sticky top-0 border-b border-zinc-800">
            <tr>
              <th className="py-2.5 px-3">Status</th>
              <th className="py-2.5 px-3">Category</th>
              <th className="py-2.5 px-3">Test Case</th>
              <th className="py-2.5 px-3">Expected Result</th>
              <th className="py-2.5 px-3">Actual Result</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-zinc-800/70">
            {testResults.map((t, idx) => (
              <tr key={idx} className="hover:bg-zinc-900/50 transition-colors">
                <td className="py-2 px-3">
                  {t.passed ? (
                    <span className="flex items-center gap-1 text-emerald-400 font-bold text-[11px]">
                      <CheckCircle2 className="w-3.5 h-3.5" />
                      PASS
                    </span>
                  ) : (
                    <span className="flex items-center gap-1 text-rose-400 font-bold text-[11px]">
                      <XCircle className="w-3.5 h-3.5" />
                      FAIL
                    </span>
                  )}
                </td>
                <td className="py-2 px-3 text-zinc-400 text-[11px] whitespace-nowrap">{t.category}</td>
                <td className="py-2 px-3 font-semibold text-zinc-200">{t.name}</td>
                <td className="py-2 px-3 text-cyan-400/90 font-mono text-[11px]">{t.expected}</td>
                <td className="py-2 px-3 text-zinc-300 font-mono text-[11px]">
                  {t.actual}
                  {t.details && <span className="block text-[10px] text-zinc-500">{t.details}</span>}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};
