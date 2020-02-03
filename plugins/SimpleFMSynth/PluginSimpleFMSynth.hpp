/*
 * Simple Synth audio efffect based on DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2018 Christopher Arndt <info@chrisarndt.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PLUGIN_SIMPLESYNTH_H
#define PLUGIN_SIMPLESYNTH_H
#define GAMMA_H_INC_ALL

#include "DistrhoPlugin.hpp"
#include "DistrhoPluginUtils.hpp"
#include "ADSR.hpp"
#include "LFO.hpp"
#include "Gamma/Gamma.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

#define SEMITONE 1.0005777895065548f

class PluginSimpleFMSynth : public Plugin {
public:
    enum Parameters {
        paramVolume = 0,
        paramFMRatio,
        paramFMAmount,
        paramAmpEnvAttack,
        paramAmpEnvDecay,
        paramAmpEnvSustain,
        paramAmpEnvRelease,
        paramFEnvAttack,
        paramFEnvDecay,
        paramFEnvSustain,
        paramFEnvRelease,
        paramLFOWaveshape,
        paramLFOFrequency,
        paramLFOOscAmount,
        paramCount
    };

    PluginSimpleFMSynth();

    ~PluginSimpleFMSynth();
protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override {
        return "Simple FM Synth";
    }

    const char* getDescription() const override {
        return "A simple FM synthesizer";
    }

    const char* getMaker() const noexcept override {
        return "chrisarndt.de";
    }

    const char* getHomePage() const override {
        return "http://chrisarndt.de/plugins/simplefmsynth";
    }

    const char* getLicense() const noexcept override {
        return "GPL";
    }

    uint32_t getVersion() const noexcept override {
        return d_version(0, 1, 0);
    }

    // Go to:
    //
    // http://service.steinberg.de/databases/plugin.nsf/plugIn
    //
    // Get a proper plugin UID and fill it in here!
    int64_t getUniqueId() const noexcept override {
        return d_cconst('s', 'F', 'M', 's');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    void loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // Optional

    // Optional callback to inform the plugin about a sample rate change.
    void sampleRateChanged(double newSampleRate) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void note_on(int8_t note, bool retrigger);
    void run(const float**, float** outputs, uint32_t frames,
             const MidiEvent *midiEvents, uint32_t midiEventCount) override;

    // -------------------------------------------------------------------

private:
    float           fParams[paramCount];
    double          fSampleRate;
    ADSR            *ampenv, *fenv;
    LFO             *lfo;
    gam::Sine<>     car, mod;
    bool            noteState[128];
    int8_t          noteStack[128];
    int8_t          noteStackPos;
    float           noteFreqs[128];
    float           curNoteFreq;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginSimpleFMSynth)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // #ifndef PLUGIN_SIMPLESYNTH_H
