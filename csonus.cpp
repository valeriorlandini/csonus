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

    int init()
    {
        t = new uint32_t(0);
        sample_count = new uint8_t(0);
        return OK;
    }

    int32_t aperf()
    {
        csnd::AudioSig out(this, outargs(0), true);

        MYFLT amp = inargs[0];  
        int formula_idx = static_cast<int>(inargs[1]);
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

    int init()
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
        MYFLT mode = inargs[3];
        MYFLT lowcut = inargs[4];

        cv->set_mode((unsigned int)mode);
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

    int init()
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

    int init()
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

    int init()
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
        MYFLT waveform = inargs[3];
        MYFLT window = inargs[4];

        pulsar->set_frequency(freq);
        pulsar->set_duty_cycle(duty_cycle);
        pulsar->set_waveform((soutel::PulsarWaveforms)(int)waveform);
        pulsar->set_window((soutel::PulsarWindows)(int)window);

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

    int init()
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


// Registration functions
#ifdef BUILD_PLUGINS
#include <modload.h>
void csnd::on_load(csnd::Csound *csound)
{
    csnd::plugin<BitInv>(csound, "bitinv", "a", "akkkkkkkk", csnd::thread::a);
    csnd::plugin<BytePlay>(csound, "byteplay", "a", "kkk", csnd::thread::ia);
    csnd::plugin<Cheby>(csound, "cheby", "a", "ak", csnd::thread::a);
    csnd::plugin<Cryptoverb>(csound, "cryptoverb", "aa", "aakkk", csnd::thread::ia);
    csnd::plugin<Lorenz>(csound, "lorenz", "aaa", "kkkkk", csnd::thread::ia);
    csnd::plugin<Perceptron>(csound, "perceptron", "a", "akk", csnd::thread::a);
    csnd::plugin<PhaseDist>(csound, "phasedist", "a", "kk", csnd::thread::ia);
    csnd::plugin<Pulsar>(csound, "pulsar", "a", "kkkk", csnd::thread::ia);
    csnd::plugin<QuadPan>(csound, "quadpan", "aaaa", "akk", csnd::thread::a);
    csnd::plugin<Roessler>(csound, "roessler", "aaa", "kkkkk", csnd::thread::ia);
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
    csnd::plugin<Lorenz>((csnd::Csound *)csound, "lorenz", "aaa", "kkkkk",
                         csnd::thread::ia);
    csnd::plugin<Perceptron>((csnd::Csound *)csound, "perceptron", "a", "akk",
                             csnd::thread::a);
    csnd::plugin<PhaseDist>((csnd::Csound *)csound, "phasedist", "a", "kk",
                            csnd::thread::ia);
    csnd::plugin<Pulsar>((csnd::Csound *)csound, "pulsar", "a", "kkkk",
                         csnd::thread::ia);
    csnd::plugin<QuadPan>((csnd::Csound *)csound, "quadpan", "aaaa", "akk",
                          csnd::thread::a);
    csnd::plugin<Roessler>((csnd::Csound *)csound, "roessler", "aaa", "kkkkk",
                           csnd::thread::ia);
    return OK;
}
#endif