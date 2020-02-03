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

#include <math.h>

#include "PluginSimpleFMSynth.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

PluginSimpleFMSynth::PluginSimpleFMSynth()
    : Plugin(paramCount, 1, 0)  // paramCount params, 1 program(s), 0 states
{
    ampenv = new ADSR();
    fenv = new ADSR();
    lfo = new LFO(getSampleRate());
    loadProgram(0);
}

PluginSimpleFMSynth::~PluginSimpleFMSynth() {
    delete ampenv;
    delete fenv;
    delete lfo;
}

// -----------------------------------------------------------------------
// Init

void PluginSimpleFMSynth::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;

    ParameterEnumerationValue* const wforms = new ParameterEnumerationValue[LFO::kNumWave];
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.1f;
    parameter.hints = kParameterIsAutomable;

    switch (index) {
        case paramVolume:
            parameter.name = "Volume";
            parameter.symbol = "volume";
            parameter.ranges.def = 0.4f;
            parameter.hints |= kParameterIsLogarithmic;
            break;
        case paramFMRatio:
            parameter.name = "FM Ratio";
            parameter.symbol = "fm_ratio";
            parameter.ranges.min = 0.5f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 1.0f;
            break;
        case paramFMAmount:
            parameter.name = "FM Amount";
            parameter.symbol = "fm_amount";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.0f;
            break;
        case paramAmpEnvAttack:
            parameter.name = "Amp. Env. Attack";
            parameter.symbol = "aenv_attack";
            parameter.unit = "s";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.01f;
            break;
        case paramAmpEnvDecay:
            parameter.name = "Amp. Env. Decay";
            parameter.symbol = "aenv_decay";
            parameter.unit = "s";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.0f;
            break;
        case paramAmpEnvSustain:
            parameter.name = "Amp. Env. Sustain";
            parameter.symbol = "aenv_sustain";
            parameter.ranges.def = 1.0f;
            break;
        case paramAmpEnvRelease:
            parameter.name = "Amp. Env. Release";
            parameter.symbol = "aenv_release";
            parameter.unit = "s";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.04f;
            break;
        case paramFEnvAttack:
            parameter.name = "F. Env. Attack";
            parameter.symbol = "fenv_attack";
            parameter.unit = "s";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.01f;
            break;
        case paramFEnvDecay:
            parameter.name = "F. Env. Decay";
            parameter.symbol = "fenv_decay";
            parameter.unit = "s";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.0f;
            break;
        case paramFEnvSustain:
            parameter.name = "F. Env. Sustain";
            parameter.symbol = "fenv_sustain";
            parameter.ranges.def = 1.0f;
            break;
        case paramFEnvRelease:
            parameter.name = "F. Env. Release";
            parameter.symbol = "fenv_release";
            parameter.unit = "s";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            parameter.ranges.def = 0.04f;
            break;
        case paramLFOWaveshape:
            parameter.name = "LFO Shape";
            parameter.symbol = "lfo_shape";
            parameter.hints |= kParameterIsInteger;
            parameter.ranges.min = 0;
            parameter.ranges.max = LFO::kNumWave - 1;
            parameter.ranges.def = LFO::triangle;
            wforms[0].label = "Triangle";
            wforms[0].value = LFO::triangle;
            wforms[1].label = "Sine";
            wforms[1].value = LFO::sinus;
            wforms[2].label = "Sawtooth";
            wforms[2].value = LFO::sawtooth;
            wforms[3].label = "Square";
            wforms[3].value = LFO::square;
            wforms[4].label = "Exponential";
            wforms[4].value = LFO::exponent;
            parameter.enumValues.count = LFO::kNumWave;
            parameter.enumValues.restrictedMode = true;
            parameter.enumValues.values = wforms;
            break;
        case paramLFOFrequency:
            parameter.name = "LFO Freq.";
            parameter.symbol = "lfo_freq";
            parameter.unit = "hz";
            parameter.ranges.min = 0.01f;
            parameter.ranges.max = 30.0f;
            parameter.ranges.def = 8.0f;
            break;
        case paramLFOOscAmount:
            parameter.name = "LFO->Osc";
            parameter.symbol = "lfo_osc_amount";
            parameter.unit = "cent";
            parameter.hints |= kParameterIsInteger;
            parameter.ranges.def = 0;
            parameter.ranges.min = -12000;
            parameter.ranges.max = 12000;
            break;
        }
}

/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void PluginSimpleFMSynth::initProgramName(uint32_t index, String& programName) {
    switch (index) {
        case 0:
            programName = "Default";
            break;
    }
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void PluginSimpleFMSynth::sampleRateChanged(double newSampleRate) {
    fSampleRate = newSampleRate;
    lfo->setSampleRate(newSampleRate);
    gam::sampleRate(newSampleRate);
}

/**
  Get the current value of a parameter.
*/
float PluginSimpleFMSynth::getParameterValue(uint32_t index) const {
    return fParams[index];
}

/**
  Change a parameter value.
*/
void PluginSimpleFMSynth::setParameterValue(uint32_t index, float value) {
    fParams[index] = value;

    switch (index) {
        case paramVolume:
            // do something when volume param is set
            break;
        case paramFMRatio:
            // do something when volume param is set
            break;
        case paramFMAmount:
            // do something when volume param is set
            break;
        case paramAmpEnvAttack:
            ampenv->setAttackRate(value * fSampleRate);
            break;
        case paramAmpEnvDecay:
            ampenv->setDecayRate(value * fSampleRate);
            break;
        case paramAmpEnvSustain:
            ampenv->setSustainLevel(value);
            break;
        case paramAmpEnvRelease:
            ampenv->setReleaseRate(value * fSampleRate);
            break;
        case paramFEnvAttack:
            fenv->setAttackRate(value * fSampleRate);
            break;
        case paramFEnvDecay:
            fenv->setDecayRate(value * fSampleRate);
            break;
        case paramFEnvSustain:
            fenv->setSustainLevel(value);
            break;
        case paramFEnvRelease:
            fenv->setReleaseRate(value * fSampleRate);
            break;
        case paramLFOWaveshape:
            lfo->setWaveform((LFO::waveform_t)value);
            break;
        case paramLFOFrequency:
            lfo->setRate(value);
            break;
        case paramLFOOscAmount:
            // nothing to do
            break;
        }
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void PluginSimpleFMSynth::loadProgram(uint32_t index) {
    switch (index) {
        case 0:
            setParameterValue(paramVolume, 0.4f);
            setParameterValue(paramFMRatio, 1.0f);
            setParameterValue(paramFMAmount, 0.0f);
            setParameterValue(paramAmpEnvAttack, 0.02f);
            setParameterValue(paramAmpEnvDecay, 0);
            setParameterValue(paramAmpEnvSustain, 1.f);
            setParameterValue(paramAmpEnvRelease, 0.02f);
            setParameterValue(paramFEnvAttack, 0.01f);
            setParameterValue(paramFEnvDecay, 0);
            setParameterValue(paramFEnvSustain, 1.0f);
            setParameterValue(paramFEnvRelease, 0.02f);
            setParameterValue(paramLFOWaveshape, LFO::triangle);
            setParameterValue(paramLFOFrequency, 8.0f);
            setParameterValue(paramLFOOscAmount, 0);
            break;
    }
}

// -----------------------------------------------------------------------
// Process

void PluginSimpleFMSynth::activate() {
    sampleRateChanged(getSampleRate());
    ampenv->reset();
    fenv->reset();
    lfo->reset();
    car.freq(440.0f);

    for (int i=0; i < 128; i++) {
        noteState[i] = false;
        noteStack[i] = -1;
        noteFreqs[i] = 440.0f * powf(2.0f, (i - 69.0f) / 12.0f);
    }

    noteStackPos = -1;
}

void PluginSimpleFMSynth::note_on(int8_t note, bool retrigger) {
    curNoteFreq = noteFreqs[note];

    if (retrigger) {
        ampenv->gate(true);
        fenv->gate(true);
    }
}

void PluginSimpleFMSynth::run(const float**, float** outputs, uint32_t frames,
                            const MidiEvent *midiEvents, uint32_t midiEventCount) {
    uint8_t note, velo;
    float cutoff, fenvval, lfoval, oscfreq, vol = fParams[paramVolume];

    // Loop over MIDI events in this block in batches grouped by event frame number,
    // i.e. each batch has a number of events occuring at the same frame number.
    // First we need to make an instance of sync helper in the for-loop initializer.
    // The for-loop break condition is amsh.nextEvent(), when it returns null,
    // there are no more events and the loop ends.
    //
    // One could also use a while-loop like this:
    //
    // AudioMidiSyncHelper amsh(outputs, frames, midiEvents, midiEventCount);
    //
    // while (amsh.nextEvent()) {
    //    ...
    // }
    for (AudioMidiSyncHelper amsh(outputs, frames, midiEvents, midiEventCount); amsh.nextEvent();) {
        // Loop over events in this batch
        for (uint32_t i=0; i<amsh.midiEventCount; ++i) {
            if (amsh.midiEvents[i].size > MidiEvent::kDataSize)
                // Skip MIDI events with more than 3 bytes.
                continue;

            const uint8_t* data = amsh.midiEvents[i].data;
            // Determine MIDI event status by masking out MIDI channel in lower four bits.
            const uint8_t status = data[0] & 0xF0;

            switch (status) {
                case 0x90:
                    // Note On
                    note = data[1];
                    velo = data[2];
                    // Make sure note number is within range 0 .. 127
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (velo > 0) {
                        noteState[note] = true;

                        if (noteStackPos >= 0 && note == noteStack[noteStackPos])
                            // XXX: re-trigger envelopes here?
                            break;

                        note_on(note, noteStackPos == -1);
                        noteStack[++noteStackPos] = note;
                        break;
                    }
                    // for Note On with velocity 0 => Note Off
                    // fall through
                case 0x80:
                    note = data[1];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);
                    noteState[note] = false;

                    // No note currently playing
                    if (noteStackPos == -1)
                        break;

                    // Note off for currently playing note
                    if (noteStack[noteStackPos] == note) {
                        // While note stack is not empty and previous held note already released
                        while (noteStackPos >= 0 && !noteState[note]) {
                            // Get next previously held note
                            note = noteStack[--noteStackPos];
                        }

                        if (noteStackPos >= 0) {
                            // A note is still held
                            note_on(note, false);
                        }
                        else {
                            // No notes held anymore
                            ampenv->gate(false);
                            fenv->gate(false);
                        }
                    }

                    break;
            }
        }

        // Get the left and right audio outputs from the AudioMidiSyncHelper.
        // This ensures that the pointer is positioned at the right frame within
        // the block for the current batch.
        float* const outL = amsh.outputs[0];
        float* const outR = amsh.outputs[1];

        float modfreq = curNoteFreq * fParams[paramFMRatio];

        // Write samples for the frames of the current batch
        for (uint32_t i=0; i<amsh.frames; ++i) {
            lfoval = lfo->tick();
            fenvval = fenv->process();
            oscfreq = curNoteFreq * pow(SEMITONE, fParams[paramLFOOscAmount] * lfoval);
            mod.freq(modfreq);
            car.freq(oscfreq + mod() * fParams[paramFMAmount] * fenvval * modfreq);
            //car.freq(oscfreq);
            float sample = car() * ampenv->process() * vol;
            outL[i] = sample;
            outR[i] = sample;
        }
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new PluginSimpleFMSynth();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
