/******************************************************************************
Copyright (c) 2026 Valerio Orlandini

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <plugin.h>
#include <cmath>

#include "include/genealgo.h"
#include "soutel/include/soutel/soutel.h"


/******************************************************************************
bitinv: Invert specific bits of an audio signal.

This opcode takes an audio signal and inverts specific bits of its 8-bit
representation based on user-specified parameters. The input signal is first
scaled to fit within the range of 8 bits (0-255), and then the specified bits
are inverted using bitwise XOR operations. Finally, the modified signal is
scaled back to the original audio range (-1.0 to 1.0).

Control Parameters:
  bit1, bit2, ..., bit8: Boolean parameters (0 or 1) that determine which
  bits to invert. For example, if bit1 is set to 1, the least significant bit
  will be inverted. If bit8 is set to 1, the most significant bit will be
  inverted.

Usage:
  a bitinv in, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8
******************************************************************************/
struct BitInv : csnd::Plugin<1, 9>
{
    int32_t aperf()
    {
        csnd::AudioSig out(this, outargs(0), true);
        csnd::AudioSig in(this, inargs(0));

        MYFLT bit1 = inargs[1];
        MYFLT bit2 = inargs[2];
        MYFLT bit3 = inargs[3];
        MYFLT bit4 = inargs[4];
        MYFLT bit5 = inargs[5];
        MYFLT bit6 = inargs[6];
        MYFLT bit7 = inargs[7];
        MYFLT bit8 = inargs[8];

        auto in_it = in.begin();
        for (auto &s : out)
        {
            s = *in_it++;

            if (s < -1.0)
            {
                s = -1.0;
            }
            if (s > 1.0)
            {
                s = 1.0;
            }

            // Reduce input to 8 bit resolution
            uint8_t in = (uint8_t)round((1.0 + s) * 0.5 * 255.0);

            bool bit_status[8] = {bit1 > 0.5, bit2 > 0.5, bit3 > 0.5, bit4 > 0.5,
                                  bit5 > 0.5, bit6 > 0.5, bit7 > 0.5, bit8 > 0.5
                                 };

            for (int b = 0; b < 8; b++)
            {
                if (bit_status[7 - b])
                {
                    in ^= 1 << b;
                }
            }

            // Scale back to -1.0 to 1.0
            s = (in / 255.0) * 2.0 - 1.0;

        }
        return OK;
    }
};


/******************************************************************************
byteplay: Bytebeat formulas player.

This opcode generates a classic bytebeat waveform with user-specified
parameters. Bytebeat is a form of algorithmic music that uses simple
mathematical formulas to generate audio signals. The output consists of
a single audio signal that evolves over time based on the specified
formula. Users can explore a wide variety of soundscapes by adjusting
the formula and its parameters.

Control Parameters:
  amp: The amplitude of the output signal.
  formula_idx: The index of the bytebeat formula to use (0-15).
  sr_red: Sample rate reduction factor, which controls the speed of the
  evolution of the bytebeat pattern. Higher values will result in slower
  changes in the output signal, while lower values will create faster, more
  rapidly evolving patterns.

Usage:
  a byteplay amp, formula_idx, sr_red
*********************************************************************************/
struct BytePlay : csnd::Plugin<1, 3>
{
    uint32_t *t = nullptr;
    uint8_t *sample_count = nullptr;

    int32_t init()
    {
        t = new uint32_t(0);
        sample_count = new uint8_t(0);
        return OK;
    }

    int32_t aperf()
    {
        csnd::AudioSig out(this, outargs(0), true);

        MYFLT amp = inargs[0];  
        uint8_t formula_idx = static_cast<uint8_t>(inargs[1]);
        if (formula_idx > 15)
        {
            return csound->perf_error("Formula index out of range.", this);
        }

        uint8_t sr_red = static_cast<uint8_t>(inargs[2]);

        MYFLT last_out = 0.0;

        for (auto &s : out)
        {
            ++(*sample_count);

            if (*sample_count >= sr_red)
            {
                ++(*t);
                uint8_t out = 0;

                switch (formula_idx)
                {
                case 0:
                    out = *t;
                    break;
                case 1:
                    out = *t*5 & *t>>7;
                    break;
                case 2:
                    out = (*t*9 & *t>>4 | *t*5 & *t>>7 | *t*3 & *t>>10) - 1;
                    break;
                case 3:
                    out = (*t>>8 & *t) * (*t>>15 & *t);
                    break;
                case 4:
                    out = (*t%(*t>>8|*t>>16))^*t;
                    break;
                case 5:
                    out = (*t%255&*t)-(*t>>13&*t);
                    break;
                case 6:
                    out = (*t-(*t>>4&*t>>8)&*t>>12)-1;
                    break;
                case 7:
                    out = ((*t**t)/(*t^*t>>8))&*t;
                    break;
                case 8:
                    out = ((2*(*t&1)-1)*(*t))-(*t>>8);
                    break;
                case 9:
                    out = *t*(*t>>(*t>>13&*t));
                    break;
                case 10:
                    out = (*t**t/(1+(*t>>9&*t>>8)))&128;
                    break;
                case 11:
                    out = (*t*(-(*t>>8|*t|*t>>9|*t>>13)))^*t;
                    break;
                case 12:
                    out = (*t<<13)|(*t&*t>>3)*(*t>>11&*t)-(*t>>(*t^17))-2;
                    break;
                case 13:
                    out = (*t&*t>>4)-(*t>>13&*t);
                    break;
                case 14:
                    out = ((*t/1000)^(*t/1001))*(*t);
                    break;
                case 15:
                    out = (((*t&*t>>8)-(*t>>13&*t))&((*t&*t>>8)-(*t>>13)))^(*t>>8&*t);
                    break;
                }

                s = ((static_cast<MYFLT>(out) - 127.5) / 127.5) * amp;
                last_out = s;
                *sample_count = 0;
            }
            else
            {
                s = last_out;
            }
        }

        return OK;
    }
};


/******************************************************************************
cheby: Invert specific bits of an audio signal.

This opcode takes an audio signal and applies a Chebyshev polynomial of a
specified order to it. The input signal is processed through the Chebyshev
function, which can create a variety of distortion effects depending on
the order parameter. The order parameter determines the degree of the Chebyshev
polynomial applied to the input signal, with higher orders resulting in more
complex and harmonically rich distortions.

Control Parameters:
  order: The order of the Chebyshev polynomial (non integer values will lead
  to an interpolation between the two closest integer orders, according
  to the fractional part of the parameter) (0-10).

Usage:
  a cheby in, order
******************************************************************************/
struct Cheby : csnd::Plugin<1, 2>
{
    int32_t aperf()
    {
        csnd::AudioSig out(this, outargs(0), true);
        csnd::AudioSig in(this, inargs(0));

        MYFLT order = inargs[1];
        if (order < 0.0 || order > 10.0)
        {
            return csound->perf_error("Currently supported order range is 0.0 to 10.0.", this);
        }

        auto in_it = in.begin();
        for (auto &s : out)
        {
            s = *in_it++;

            s = soutel::chebyshev(s, order);
        }
        return OK;
    }
};


/******************************************************************************
cryptoverb: Eerie stereo reverberation effects with three different block
processing modes.

This opcode takes an audio signal and applies a series of reverberation effects
based on user-specified parameters. The input signal is processed with a
series of allpass and comb filter, with an arrangement that can be changed
based on the mode parameter, yelding different reverberation characteristics.
The lowpass cutoff parameter controls the overall brightness of the reverb.

Control Parameters:
  wet: The wet mix level (0.0 to 1.0).
  mode: The processing mode (0, 1 or 2).
  lowcut: The lowcut frequency (in Hz).

Usage:
  al, ar cryptoverb in_l, in_r, wet, mode, lowcut
******************************************************************************/
struct Cryptoverb : csnd::Plugin<2, 5>
{
    soutel::Cryptoverb<MYFLT> *cv;

    int32_t init()
    {
        cv = new soutel::Cryptoverb<MYFLT>();
        cv->set_sample_rate(this->sr());
        return OK;
    }

    int32_t aperf()
    {
        csnd::AudioSig out_l(this, outargs(0), true);
        csnd::AudioSig out_r(this, outargs(1), true);
        csnd::AudioSig in_l(this, inargs(0));
        csnd::AudioSig in_r(this, inargs(1));

        MYFLT wet = inargs[2];
        unsigned int mode = static_cast<unsigned int>(inargs[3]);
        if (mode > 2)
        {
            return csound->perf_error("Mode parameter out of range.", this);
        }

        MYFLT lowcut = inargs[4];

        cv->set_mode(mode);
        cv->set_lowpass_cutoff(lowcut);

        for (uint32_t i = offset; i < nsmps; i++)
        {
            auto output = cv->run(in_l[i], in_r[i]);
            out_l[i] = output[0] * wet + in_l[i] * (1.0 - wet);
            out_r[i] = output[1] * wet + in_r[i] * (1.0 - wet);
        }

        return OK;
    }
};


/******************************************************************************
genetic: A system implementing a genetic algorithm to evolve a string sequence
towards a target string.

This opcode implements a genetic algorithm that evolves a population of string
sequences towards a target string defined by the user. The algorithm starts
with a randomly populated population of strings, and iteratively applies
selection, crossover, and mutation operations to evolve the population towards
the target. The mutation rate and mating rate parameters control the frequency
of mutation and crossover events, respectively. The output is the best matching
string from the population at each control, which evolves over time as the
algorithm progresses.

Control Parameters:
  target: The target string to evolve towards (a string).
  dictionary: The set of characters allowed in the evolved strings (a string).
  population_size: The number of strings in the population (an integer).
  mutation_rate: The probability of a character mutating (a float).
  mating_rate: The probability of two strings exchanging genetic material (a float).
  multiple_crossover: Whether to allow multiple crossover points (0 or 1, default: 0).

Usage:
  S genetic target, dictionary, population_size, mutation_rate, mating_rate, multiple_crossover
******************************************************************************/
struct Genetic : csnd::Plugin<1, 6>
{
    GeneAlgo<char> *ga;
    bool *multiple_crossover = nullptr;
    bool *target_reached = nullptr;

    int32_t init()
    {
        auto target = inargs.str_data(0).data;
        auto dictionary = inargs.str_data(1).data;
        unsigned int population_size = std::max(2u, static_cast<unsigned int>(inargs[2]));
        float mutation_rate = inargs[3];
        float mating_rate = inargs[4];
        multiple_crossover = new bool(inargs[5] > 0.5);
        target_reached = new bool(false);

        ga = new GeneAlgo<char>();
        if (!ga->populate(std::vector<char>(dictionary, dictionary + std::strlen(dictionary)),
                          population_size, std::strlen(target)))
        {
            return csound->perf_error("Failed to populate genetic pool", this);
        }

        ga->set_target(std::vector<char>(target, target + std::strlen(target)), true);
        ga->set_mutation_rate(mutation_rate);
        ga->set_mating_rate(mating_rate);

        return OK;
    }

    int32_t kperf()
    {
        if (!*target_reached)
        {
            *target_reached = ga->evolution(*multiple_crossover);
        }

        auto population = ga->get_population();
        std::vector<char> target = population.at(0).sequence;
        std::string target_str(target.begin(), target.end());

        char* ns = new char[target_str.size() + 1];
        std::strcpy(ns, target_str.c_str());
        char *result = csound->strdup(ns);
        if (result == nullptr)
        {
            return csound->perf_error("Memory allocation failed", this);
        }
        outargs.str_data(0).data = result;
        return OK;
    }
};


/******************************************************************************
linden: A configurable Lindenmayer system.

This opcode takes a string representing the initial sequence and a set of
production rules, and applies the rules iteratively to generate a new sequence
at each control, which is then output as a string. The production rules ar
defined in the format "A>AB,B>AC,C>BAA", where the character before the ">" is
the input (one single character) and the string after the ">" is the output
that will replace it (and may be a string of any length). The maximum string
length parameter allows users to limit the length of the output sequence, which
can be useful to prevent excessively long outputs as the system evolves.

Control Parameters:
  axiom: The initial value for the Lindenmayer system (a string).
  rules: The production rules for the Lindenmayer system (a string).
  max_length: The maximum length of the output sequence (an integer).

Usage:
  S linden axiom, rules, max_length
******************************************************************************/
struct Linden : csnd::Plugin<1, 3>
{
    struct rule
    {
        char input;
        std::string output;
    };

    std::string *sequence = nullptr;
    std::vector<rule> *rules;
    unsigned int *max_string_length = nullptr;

    int32_t init()
    {
        sequence = new std::string(inargs.str_data(0).data);
        rules = new std::vector<rule>();
        auto all_rules = split(inargs.str_data(1).data, ',');

        max_string_length = new unsigned int(inargs[2]);

        for (auto &r : all_rules)
        {
            auto curr_rule = split(r, '>');
            if (curr_rule.size() > 1)
            {
                rule curr_rule_struct;
                curr_rule_struct.input = curr_rule[0].at(0);
                curr_rule_struct.output = curr_rule[1];
                rules->push_back(curr_rule_struct);
            }
        }

        return OK;
    }

    int32_t kperf()
    {
        std::string next_sequence = "";

        for (unsigned int i = 0; i < sequence->size(); i++)
        {
            char c = sequence->at(i);
            bool rule_applied = false;
            for (auto r = 0; r < rules->size(); r++)
            {
                if (c == rules->at(r).input)
                {
                    next_sequence += rules->at(r).output;
                    rule_applied = true;
                    break;
                }
            }
            if (!rule_applied)
            {
                next_sequence += c;
            }
        }

        if (*max_string_length > 0 && next_sequence.size() > *max_string_length)
        {
            next_sequence = next_sequence.substr(next_sequence.size() - *max_string_length);
        }

        char* ns = new char[next_sequence.size() + 1];
        std::strcpy(ns, next_sequence.c_str());
        char *result = csound->strdup(ns);
        if (result == nullptr)
        {
            return csound->perf_error("Memory allocation failed", this);
        }
        outargs.str_data(0).data = result;
        *sequence = next_sequence;
        return OK;
    }

    std::vector<std::string> split(const std::string& str, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delimiter))
        {
            result.push_back(item);
        }

        return result;
    }
};


/******************************************************************************
logistic: A control rate opcode that applies the classical logistic map algorithm.

This opcode takes a control signal and applies a simple logistic map algorithm to
it. The input signal is multiplied by a weight and then added to a bias. The
result is then processed through the logistic map function, which creates a chaotic
output signal. The logistic map is a piecewise linear function that can generate
complex and chaotic behavior, making it useful for creating evolving modulation
sources or for generating unique textures in a control signal.

Control Parameters:
  start: The initial value for the logistic map (0.0 to 1.0).
  r: The r parameter for the logistic map, which controls the shape of the function
  and the degree of chaos in the output signal (0.0 to 3.999, with values from 3.57
  onwards leading to chaotic behavior).

Usage:
  k logistic start, r
******************************************************************************/
struct Logistic : csnd::Plugin<1, 2>
{
    MYFLT *out = nullptr;

    int32_t init()
    {
        out = new MYFLT(std::clamp(inargs[0], 0.0, 1.0));
        return OK;
    }

    int32_t kperf()
    {
        MYFLT r = std::clamp(inargs[1], 0.0, 3.9999);

        *out = r * *out * (1.0 - *out);

        outargs[0] = *out;

        return OK;
    }
};


/******************************************************************************
lorenz: Lorenz attractor.

This opcode generates a classic Lorenz attractor waveform with user-specified
parameters. The Lorenz attractor is a system of three ordinary differential
equations that exhibits chaotic behavior. The output consists of three audio
signals corresponding to the x, y, and z coordinates of the attractor.
The speed of the attractor's evolution is controlled by the speed parameter,
while the shape of the attractor is determined by the beta, sigma, and rho
parameters. By adjusting these parameters, users can explore a wide variety
of chaotic trajectories, creating complex and evolving soundscapes, ideal as
modulation sources or for generating unique textures.

Control Parameters:
  amp: The amplitude of the output signals.
  speed: The speed of the attractor's evolution.
  beta: The beta parameter.
  sigma: The sigma parameter.
  rho: The rho parameter.

Usage:
  ax, ay, az lorenz amp, speed, beta, sigma, rho
*********************************************************************************/
struct Lorenz : csnd::Plugin<3, 4>
{
    soutel::Lorenz<MYFLT> *lrnz;

    int32_t init()
    {
        lrnz = new soutel::Lorenz<MYFLT>();
        return OK;
    }

    int aperf()
    {
        csnd::AudioSig x(this, outargs(0));
        csnd::AudioSig y(this, outargs(1));
        csnd::AudioSig z(this, outargs(2));
        MYFLT amp = inargs[0];
        MYFLT speed = inargs[1];
        MYFLT beta = inargs[2];
        MYFLT sigma = inargs[3];
        MYFLT rho = inargs[4];

        lrnz->set_t(speed * 0.02);
        lrnz->set_beta(beta);
        lrnz->set_sigma(sigma);
        lrnz->set_rho(rho);

        for (uint32_t i = offset; i < nsmps; i++)
        {
            lrnz->step();

            x[i] = std::clamp(lrnz->get_x() * 0.04, -1.0, 1.0) * amp;
            y[i] = std::clamp(lrnz->get_y() * 0.04, -1.0, 1.0) * amp;
            z[i] = std::clamp((lrnz->get_z() * 0.04) - 1.0, -1.0, 1.0) * amp;
        }

        return OK;
    }
};


/******************************************************************************
neurosc: Neural waveform generator.

This opcode generates a neural waveform with user-specified parameters. 
The output consists of a single audio signal generated by the neural network,
an autoencoder with eight latent variables, which can be modified by
the user through the control parameters, allowing for a wide range of timbral
possibilities and smoothly evolving shapes.

Control Parameters:
  amp: The amplitude of the output signals.
  freq: The frequency of the neural waveform.
  w1-w8: The weights for the latent variables.
  window: A flag to enable windowing of the waveform.

Usage:
  a neurosc amp, freq, w1, w2, w3, w4, w5, w6, w7, w8, window
*********************************************************************************/
struct Neurosc : csnd::Plugin<1, 11>
{
    soutel::NeuralWave<MYFLT> *osc;

    int32_t init()
    {
        osc = new soutel::NeuralWave<MYFLT>();
        osc->set_sample_rate(this->sr());
        return OK;
    }

    int aperf()
    {
        csnd::AudioSig out(this, outargs(0));

        MYFLT amp = inargs[0];
        MYFLT freq = inargs[1];
        std::array<MYFLT, 8> weights;
        for (int i = 0; i < 8; i++)
        {
            weights[i] = inargs[2 + i];
        }
        bool window = inargs[10] > 0.5;

        osc->set_frequency(freq);
        osc->set_latent_space(weights);
        osc->set_windowed(window);

        for (auto &s : out)
        {
            s = osc->run() * amp;
        }

        return OK;
    }
};


/******************************************************************************
perceptron: A simple operator that applies the classical perceptron algorithm
to an audio signal.

This opcode takes an audio signal and applies a simple perceptron algorithm to
it. The input signal is multiplied by a weight and then added to a bias. The
result is then passed through a step function, which outputs 1.0 if the result
is greater than 0, and 0.0 otherwise. This can be used to create a simple
thresholding effect, where the output is either on or off based on the input
signal and the specified weight and bias parameters.

Control Parameters:
  weight: The weight for the input signal.
  bias: The bias for the output signal.

Usage:
  a perceptron in, weight, bias
******************************************************************************/
struct Perceptron : csnd::Plugin<1, 3>
{
    int32_t aperf()
    {
        csnd::AudioSig out(this, outargs(0), true);
        csnd::AudioSig in(this, inargs(0));

        MYFLT weight = inargs[1];
        MYFLT bias = inargs[2];

        auto in_it = in.begin();
        for (auto &s : out)
        {
            s = *in_it++ * weight + bias;
            s = s > 0.0 ? 1.0 : 0.0;
        }
        return OK;
    }
};


/******************************************************************************
phasedist: phase distortion oscillator.

This opcode generates a classic phase distortion waveform with user-specified
distortion parameter. The frequency of the oscillator is controlled by the freq
parameter, while the shape of the waveform is determined by the distortion
parameter d. The phase distortion algorithm modifies the phase of a basic
oscillator to create a wide variety of timbres, from simple sine waves to more
complex and harmonically rich sounds.

Control Parameters:
  amp: The amplitude of the output signal.
  freq: The frequency of the oscillator.
  d: The phase distortion parameter.

Usage:
  a phasedist amp, freq, d
*********************************************************************************/
struct PhaseDist : csnd::Plugin<1, 4>
{
    soutel::PDOsc<MYFLT> *pd;

    int32_t init()
    {
        pd = new soutel::PDOsc<MYFLT>();
        pd->set_sample_rate(this->sr());
        return OK;
    }

    int aperf()
    {
        csnd::AudioSig out(this, outargs(0));
        MYFLT amp = inargs[0];
        MYFLT freq = inargs[1];
        MYFLT d = inargs[2];

        pd->set_frequency(freq);
        pd->set_d(d);

        for (auto &s : out)
        {
            s = pd->run() * amp;
        }
        return OK;
    }
};


/******************************************************************************
pulsar: Pulsar oscillator, as described in "Microsound" by Curtis Roads.

This opcode generates a pulsar waveform based on user-specified parameters such
as frequency, duty cycle, waveform type, and window type. The pulsar is a
short burst of sound that can be shaped using different waveforms and windows to
create a wide variety of timbres. The frequency and duty cycle control the
rate and duration of the pulsar bursts, while the waveform and window types
shape the sound of each burst.

Control Parameters:
  amp: The amplitude of the output signal.
  freq: The frequency of the oscillator.
  duty_cycle: The duty cycle of the pulsar waveform.
  waveform: The type of waveform to generate.
  window: The type of window to apply.

Usage:
  a pulsar amp, freq, duty_cycle, waveform, window
*********************************************************************************/
struct Pulsar : csnd::Plugin<1, 4>
{
    soutel::Pulsar<MYFLT> *pulsar;

    int32_t init()
    {
        pulsar = new soutel::Pulsar<MYFLT>();
        pulsar->set_sample_rate(this->sr());
        return OK;
    }

    int aperf()
    {
        csnd::AudioSig out(this, outargs(0));
        MYFLT freq = inargs[0];
        MYFLT amp = inargs[1];
        MYFLT duty_cycle = inargs[2];
        int waveform = static_cast<int>(inargs[3]);
        if (waveform < 0 || waveform > 6)
        {
            return csound->perf_error("Waveform parameter out of range.", this);
        }
        int window = static_cast<int>(inargs[4]);
        if (window < 0 || window > 12)
        {
            return csound->perf_error("Window parameter out of range.", this);
        }

        pulsar->set_frequency(freq);
        pulsar->set_duty_cycle(duty_cycle);
        pulsar->set_waveform((soutel::PulsarWaveforms)waveform);
        pulsar->set_window((soutel::PulsarWindows)window);

        for (auto &s : out)
        {
            s = pulsar->run() * amp;
        }
        return OK;
    }
};


/******************************************************************************
quadpan: Quadraphonic panning of an audio signal.

This opcode takes an audio signal and two control parameters (x and y) that
determine the panning position in a 2D space. The input signal is distributed
to four output channels (front left, front right, rear left, rear right) based
on the cosine of the control parameters, creating a smooth panning effect.
The x parameter controls the left-right panning, while the y parameter controls
the front-rear panning.

Control Parameters:
  x, y: Control parameters (0.0 to 1.0) that determine the panning position in a 2D space.

Usage:
  afl, afr, arl, arr quadpan in, x, y
******************************************************************************/
struct QuadPan : csnd::Plugin<4, 3>
{
    int32_t aperf()
    {
        const MYFLT pi = 3.14159265358979323846;
        const MYFLT pi_2 = pi * 0.5;
        MYFLT *fl = outargs(0);
        MYFLT *fr = outargs(1);
        MYFLT *rl = outargs(2);
        MYFLT *rr = outargs(3);
        MYFLT *in = inargs(0);
        MYFLT x = inargs[1];
        MYFLT y = inargs[2];

        for (uint32_t i = offset; i < nsmps; i++)
        {
            fl[i] = in[i] * cos(x * pi_2) * cos(y * pi_2);
            fr[i] = in[i] * cos((1.0 - x) * pi_2) * cos(y * pi_2);
            rl[i] = in[i] * cos(x * pi_2) * cos((1.0 - y) * pi_2);
            rr[i] = in[i] * cos((1.0 - x) * pi_2) * cos((1.0 - y) * pi_2);
        }

        return OK;
    }
};


/******************************************************************************
roessler: Roessler attractor.

This opcode generates a classic Roessler attractor waveform with user-specified
parameters. The Roessler attractor is a system of three ordinary differential
equations that exhibits chaotic behavior. The output consists of three audio
signals corresponding to the x, y, and z coordinates of the attractor.
The speed of the attractor's evolution is controlled by the speed parameter,
while the shape of the attractor is determined by the a, b, and c
parameters. By adjusting these parameters, users can explore a wide variety
of chaotic trajectories, creating complex and evolving soundscapes, ideal as
modulation sources or for generating unique textures.

Control Parameters:
  amp: The amplitude of the output signals.
  speed: The speed of the attractor's evolution.
  a: The a parameter.
  b: The b parameter.
  c: The c parameter.

Usage:
  ax, ay, az roessler amp, speed, a, b, c
*********************************************************************************/
struct Roessler : csnd::Plugin<3, 4>
{
    soutel::Roessler<MYFLT> *rsslr;

    int32_t init()
    {
        rsslr = new soutel::Roessler<MYFLT>();
        return OK;
    }

    int aperf()
    {
        csnd::AudioSig x(this, outargs(0));
        csnd::AudioSig y(this, outargs(1));
        csnd::AudioSig z(this, outargs(2));
        MYFLT amp = inargs[0];
        MYFLT speed = inargs[1];
        MYFLT a = inargs[2];
        MYFLT b = inargs[3];
        MYFLT c = inargs[4];

        rsslr->set_t(speed * 0.02);
        rsslr->set_a(a);
        rsslr->set_b(b);
        rsslr->set_c(c);

        for (uint32_t i = offset; i < nsmps; i++)
        {
            rsslr->step();

            x[i] = std::clamp(rsslr->get_x() * 0.08, -1.0, 1.0) * amp;
            y[i] = std::clamp(rsslr->get_y() * 0.08, -1.0, 1.0) * amp;
            z[i] = std::clamp((rsslr->get_z() * 0.02) - 1.0, -1.0, 1.0) * amp;
        }

        return OK;
    }
};


/******************************************************************************
tent: A control rate opcode that applies the classical tent map algorithm.

This opcode takes a control signal and applies a simple tent map algorithm to
it. The input signal is multiplied by a weight and then added to a bias. The
result is then processed through the tent map function, which creates a chaotic
output signal. The tent map is a piecewise linear function that can generate
complex and chaotic behavior, making it useful for creating evolving modulation
sources or for generating unique textures in a control signal.

Control Parameters:
  start: The initial value for the tent map (0.0 to 1.0).
  mu: The mu parameter for the tent map, which controls the shape of the function
  and the degree of chaos in the output signal (0.0 to 2.0, with values from 1.41
  onwards leading to chaotic behavior).

Usage:
  k tent start, mu
******************************************************************************/
struct Tent : csnd::Plugin<1, 2>
{
    MYFLT *out = nullptr;

    int32_t init()
    {
        out = new MYFLT(std::clamp(inargs[0], 0.0, 1.0));
        return OK;
    }

    int32_t kperf()
    {
        MYFLT mu = std::clamp(inargs[1], 0.0, 2.0);

        if (*out < 0.5)
        {
            *out *= mu;
        }
        else
        {
            *out = (1.0 - *out) * mu;
        }

        outargs[0] = *out;

        return OK;
    }
};


// Registration functions
#ifdef BUILD_PLUGINS
#include <modload.h>
void csnd::on_load(csnd::Csound *csound)
{
    csnd::plugin<BitInv>(csound, "bitinv", "a", "akkkkkkkk", csnd::thread::a);
    csnd::plugin<BytePlay>(csound, "byteplay", "a", "kkk", csnd::thread::ia);
    csnd::plugin<Cheby>(csound, "cheby", "a", "ak", csnd::thread::a);
    csnd::plugin<Cryptoverb>(csound, "cryptoverb", "aa", "aakkk", csnd::thread::ia);
    csnd::plugin<Genetic>(csound, "genetic", "S", "SSiiio", csnd::thread::ik);
    csnd::plugin<Linden>(csound, "linden", "S", "SSo", csnd::thread::ik);
    csnd::plugin<Logistic>(csound, "logistic", "k", "kk", csnd::thread::ik);
    csnd::plugin<Lorenz>(csound, "lorenz", "aaa", "kkkkk", csnd::thread::ia);
    csnd::plugin<Neurosc>(csound, "neurosc", "a", "kkkkkkkkkkk", csnd::thread::ia);
    csnd::plugin<Perceptron>(csound, "perceptron", "a", "akk", csnd::thread::a);
    csnd::plugin<PhaseDist>(csound, "phasedist", "a", "kkk", csnd::thread::ia);
    csnd::plugin<Pulsar>(csound, "pulsar", "a", "kkkk", csnd::thread::ia);
    csnd::plugin<QuadPan>(csound, "quadpan", "aaaa", "akk", csnd::thread::a);
    csnd::plugin<Roessler>(csound, "roessler", "aaa", "kkkkk", csnd::thread::ia);
    csnd::plugin<Tent>(csound, "tent", "k", "kk", csnd::thread::ik);
}
#else
extern "C" int32_t bitinv_init_modules(CSOUND *csound)
{
    csnd::plugin<BitInv>((csnd::Csound *)csound, "bitinv", "a", "akkkkkkkk",
                         csnd::thread::a);
    csnd::plugin<BytePlay>((csnd::Csound *)csound, "byteplay", "a", "kkk",
                           csnd::thread::ia);
    csnd::plugin<Cheby>((csnd::Csound *)csound, "cheby", "a", "ak", csnd::thread::a);
    csnd::plugin<Cryptoverb>((csnd::Csound *)csound, "cryptoverb", "aa", "aakkk",
                             csnd::thread::ia);
    csnd::plugin<Genetic>((csnd::Csound *)csound, "genetic", "S", "SSiiio", csnd::thread::ik);
    csnd::plugin<Linden>((csnd::Csound *)csound, "linden", "S", "SSo", csnd::thread::ik);
    csnd::plugin<Logistic>((csnd::Csound *)csound, "logistic", "k", "kk", csnd::thread::ik);
    csnd::plugin<Lorenz>((csnd::Csound *)csound, "lorenz", "aaa", "kkkkk",
                         csnd::thread::ia);
    csnd::plugin<Neurosc>((csnd::Csound *)csound, "neurosc", "a", "kkkkkkkkkkk",
                          csnd::thread::ia);
    csnd::plugin<Perceptron>((csnd::Csound *)csound, "perceptron", "a", "akk",
                             csnd::thread::a);
    csnd::plugin<PhaseDist>((csnd::Csound *)csound, "phasedist", "a", "kkk",
                            csnd::thread::ia);
    csnd::plugin<Pulsar>((csnd::Csound *)csound, "pulsar", "a", "kkkk",
                         csnd::thread::ia);
    csnd::plugin<QuadPan>((csnd::Csound *)csound, "quadpan", "aaaa", "akk",
                          csnd::thread::a);
    csnd::plugin<Roessler>((csnd::Csound *)csound, "roessler", "aaa", "kkkkk",
                           csnd::thread::ia);
    csnd::plugin<Tent>((csnd::Csound *)csound, "tent", "k", "kk", csnd::thread::ik);
    return OK;
}
#endif
