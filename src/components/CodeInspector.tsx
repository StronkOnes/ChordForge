import React, { useState } from 'react';
import { CPP_FILES_CATALOG, CppFileItem } from '../data/cppFiles';
import { FileCode, Copy, Check, Terminal, FolderTree } from 'lucide-react';

export const CodeInspector: React.FC = () => {
  const [selectedFile, setSelectedFile] = useState<CppFileItem>(CPP_FILES_CATALOG[0]);
  const [filterCategory, setFilterCategory] = useState<string>('All');
  const [copied, setCopied] = useState<boolean>(false);

  const categories = ['All', 'CMake', 'Plugin', 'Music Engine', 'MIDI Engine', 'Assembly', 'Presets', 'Tests', 'Documentation'];

  const filteredFiles = filterCategory === 'All'
    ? CPP_FILES_CATALOG
    : CPP_FILES_CATALOG.filter((f) => f.category === filterCategory);

  const handleCopyPath = () => {
    navigator.clipboard.writeText(selectedFile.path);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="w-full max-w-6xl mx-auto rounded-xl border border-zinc-800 bg-[#0d0f12] p-5 shadow-2xl">
      {/* Header */}
      <div className="flex flex-wrap items-center justify-between gap-3 border-b border-zinc-800 pb-4 mb-5">
        <div>
          <h2 className="text-lg font-bold font-mono text-white flex items-center gap-2">
            <FolderTree className="w-5 h-5 text-cyan-400" />
            C++ JUCE VST3 Codebase Explorer
          </h2>
          <p className="text-xs text-zinc-400 font-mono mt-0.5">
            Real compilable desktop audio plugin source code for Windows x64 VST3 handoff.
          </p>
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={handleCopyPath}
            className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-zinc-900 border border-zinc-700 hover:border-zinc-500 text-xs font-mono text-zinc-300 cursor-pointer"
          >
            {copied ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5" />}
            {copied ? 'Copied Path!' : 'Copy Path'}
          </button>
        </div>
      </div>

      {/* Category Tabs */}
      <div className="flex items-center gap-1.5 overflow-x-auto pb-2 mb-4 scrollbar-thin">
        {categories.map((cat) => (
          <button
            key={cat}
            onClick={() => setFilterCategory(cat)}
            className={`px-3 py-1 rounded text-xs font-mono cursor-pointer transition whitespace-nowrap ${
              filterCategory === cat
                ? 'bg-cyan-500 text-zinc-950 font-bold'
                : 'bg-zinc-900 hover:bg-zinc-800 text-zinc-400 hover:text-white border border-zinc-800'
            }`}
          >
            {cat}
          </button>
        ))}
      </div>

      {/* Main Two-Column Layout */}
      <div className="grid grid-cols-1 md:grid-cols-12 gap-4">
        {/* Left: File Tree List */}
        <div className="md:col-span-4 bg-zinc-950/80 border border-zinc-800 rounded-lg p-2.5 max-h-[520px] overflow-y-auto font-mono text-xs">
          <div className="text-[11px] font-bold text-zinc-400 px-2 py-1 mb-1 uppercase tracking-wider">
            Files ({filteredFiles.length})
          </div>
          <div className="space-y-1">
            {filteredFiles.map((file) => {
              const isSelected = selectedFile.path === file.path;
              return (
                <button
                  key={file.path}
                  onClick={() => setSelectedFile(file)}
                  className={`w-full text-left px-2.5 py-2 rounded flex items-center justify-between gap-2 transition cursor-pointer ${
                    isSelected
                      ? 'bg-cyan-950/80 text-cyan-300 border border-cyan-800/80 font-bold'
                      : 'hover:bg-zinc-900 text-zinc-400 hover:text-zinc-200'
                  }`}
                >
                  <div className="flex items-center gap-2 truncate">
                    <FileCode className={`w-3.5 h-3.5 shrink-0 ${isSelected ? 'text-cyan-400' : 'text-zinc-600'}`} />
                    <span className="truncate">{file.path.replace('ChordForge/', '')}</span>
                  </div>
                  <span className="text-[9px] uppercase px-1 rounded bg-zinc-800/90 text-zinc-400 border border-zinc-700/60 shrink-0">
                    {file.category}
                  </span>
                </button>
              );
            })}
          </div>
        </div>

        {/* Right: File Information & Build Integration Box */}
        <div className="md:col-span-8 flex flex-col space-y-3">
          {/* File Header Details */}
          <div className="bg-zinc-950/80 border border-zinc-800 rounded-lg p-4 font-mono">
            <div className="flex items-center justify-between gap-2 mb-2">
              <span className="text-sm font-bold text-white break-all">{selectedFile.path}</span>
              <span className="text-xs px-2 py-0.5 rounded bg-zinc-800 text-cyan-400 border border-zinc-700">
                {selectedFile.category}
              </span>
            </div>
            <p className="text-xs text-zinc-300 mb-3">{selectedFile.description}</p>
            <div className="text-[11px] text-zinc-400 border-t border-zinc-800/80 pt-2 flex items-center gap-4">
              <span>Standard: <strong className="text-zinc-200">C++20 / JUCE 7</strong></span>
              <span>Target: <strong className="text-zinc-200">Windows x64 VST3</strong></span>
              <span>Architecture: <strong className="text-zinc-200">Real-Time Safe</strong></span>
            </div>
          </div>

          {/* Windows Compilation Snippet */}
          <div className="bg-zinc-950 border border-zinc-800 rounded-lg p-4 font-mono text-xs">
            <div className="flex items-center gap-2 text-zinc-400 mb-2 font-bold text-[11px] uppercase tracking-wider">
              <Terminal className="w-4 h-4 text-emerald-400" />
              Windows Visual Studio 2022 Compilation Commands
            </div>
            <div className="bg-black/60 p-3 rounded border border-zinc-900 text-zinc-300 space-y-1.5 select-all overflow-x-auto text-[11px]">
              <div><span className="text-emerald-400 font-bold"># Configure CMake (VS 2022 x64 generator)</span></div>
              <div>cmake -B build -G "Visual Studio 17 2022" -A x64</div>
              <div className="pt-1"><span className="text-emerald-400 font-bold"># Build Release VST3 and Standalone Plugin</span></div>
              <div>cmake --build build --config Release --target ChordForge_VST3 ChordForge_Standalone</div>
              <div className="pt-1"><span className="text-emerald-400 font-bold"># Build & Execute Unit Tests</span></div>
              <div>cmake --build build --config Release --target ChordForgeTests</div>
              <div>build\tests\Release\ChordForgeTests.exe</div>
            </div>
          </div>

          {/* Quick Architecture Notes */}
          <div className="bg-zinc-900/60 border border-zinc-800/80 rounded-lg p-3 font-mono text-xs text-zinc-400 space-y-1">
            <div className="font-semibold text-zinc-200">Plugin Subsystem Architecture:</div>
            <ul className="list-disc list-inside space-y-0.5 text-[11px]">
              <li><strong className="text-zinc-300">Lock-Free Processing:</strong> Audio thread runs without memory allocation or file I/O.</li>
              <li><strong className="text-zinc-300">Host Synchronized:</strong> Synchronizes chord progression transitions with DAW playhead PPQ.</li>
              <li><strong className="text-zinc-300">Standard MIDI Writer:</strong> Emits Type 1 Standard MIDI with 960 PPQ resolution.</li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  );
};
