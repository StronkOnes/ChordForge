class AudioSynthEngine {
  private ctx: AudioContext | null = null;
  private masterGain: GainNode | null = null;
  private activeOscillators: { osc1: OscillatorNode; osc2: OscillatorNode; gain: GainNode }[] = [];
  private isMuted = false;

  private initContext() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
      this.ctx = new AudioCtx();

      this.masterGain = this.ctx.createGain();
      this.masterGain.gain.setValueAtTime(0.25, this.ctx.currentTime);

      // Stereo warmth filter
      const filter = this.ctx.createBiquadFilter();
      filter.type = 'lowpass';
      filter.frequency.setValueAtTime(2400, this.ctx.currentTime);
      filter.Q.setValueAtTime(1.0, this.ctx.currentTime);

      this.masterGain.connect(filter);
      filter.connect(this.ctx.destination);
    }

    if (this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  public setMuted(muted: boolean) {
    this.isMuted = muted;
    if (this.masterGain && this.ctx) {
      this.masterGain.gain.setValueAtTime(muted ? 0 : 0.25, this.ctx.currentTime);
    }
  }

  public stopAll() {
    if (!this.ctx) return;
    const now = this.ctx.currentTime;
    for (const voice of this.activeOscillators) {
      try {
        voice.gain.gain.cancelScheduledValues(now);
        voice.gain.gain.linearRampToValueAtTime(0.0001, now + 0.05);
        voice.osc1.stop(now + 0.06);
        voice.osc2.stop(now + 0.06);
      } catch {
        // Voice already stopped
      }
    }
    this.activeOscillators = [];
  }

  public playChord(frequencies: number[], durationSec = 1.4) {
    if (this.isMuted) return;
    this.initContext();
    if (!this.ctx || !this.masterGain) return;

    this.stopAll();

    const now = this.ctx.currentTime;
    const voiceCount = frequencies.length || 1;
    const voiceVolume = Math.min(0.28, 0.7 / Math.sqrt(voiceCount));

    for (const freq of frequencies) {
      if (freq <= 0 || isNaN(freq)) continue;

      const osc1 = this.ctx.createOscillator();
      const osc2 = this.ctx.createOscillator();
      const voiceGain = this.ctx.createGain();

      // Dual oscillator: Triangle + soft Sine with slight detuning
      osc1.type = 'triangle';
      osc1.frequency.setValueAtTime(freq, now);

      osc2.type = 'sine';
      osc2.frequency.setValueAtTime(freq * 1.002, now); // 3 cents detuning

      // ADSR Envelope
      const attackTime = 0.03;
      const decayTime = 0.25;
      const sustainLevel = voiceVolume * 0.75;
      const releaseTime = 0.35;
      const noteEndTime = now + durationSec;

      voiceGain.gain.setValueAtTime(0.0001, now);
      voiceGain.gain.linearRampToValueAtTime(voiceVolume, now + attackTime);
      voiceGain.gain.linearRampToValueAtTime(sustainLevel, now + attackTime + decayTime);
      voiceGain.gain.setValueAtTime(sustainLevel, noteEndTime - releaseTime);
      voiceGain.gain.linearRampToValueAtTime(0.0001, noteEndTime);

      osc1.connect(voiceGain);
      osc2.connect(voiceGain);
      voiceGain.connect(this.masterGain);

      osc1.start(now);
      osc2.start(now);
      osc1.stop(noteEndTime + 0.05);
      osc2.stop(noteEndTime + 0.05);

      this.activeOscillators.push({ osc1, osc2, gain: voiceGain });
    }
  }
}

export const audioSynth = new AudioSynthEngine();
