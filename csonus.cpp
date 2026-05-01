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


/******************************************************************************
bitinv: Invert specific bits of an audio signal.

This opcode takes an audio signal and inverts specific bits of its 8-bit
representation based on user-specified parameters. The input signal is first
scaled to fit within the range of 8 bits (0-255), and then the specified bits
are inverted using bitwise XOR operations. Finally, the modified signal is
scaled back to the original audio range (-1.0 to 1.0)

Control Parameters:
  bit1, bit2, ..., bit8: Boolean parameters (0 or 1) that determine which
  bits to invert. For example, if bit1 is set to 1, the least significant bit
  will be inverted. If bit8 is set to 1, the most significant bit will be
  inverted.

Usage:
  a bitinv a, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8
*********************************************************************************/
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
perceptron: A simple operator that applies the classical perceptron algorithm
to an audio signal.

Control Parameters:
  weight: The weight for the input signal.
  bias: The bias for the output signal.

Usage:
  a perceptron a, weight, bias
*********************************************************************************/
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
  a quadpan a, x, y
*********************************************************************************/
struct QuadPan : csnd::Plugin<4, 3>
{
    constexpr pi = 3.14159265358979323846;
    constexpr pi_2 = pi * 0.5;

    int32_t aperf()
    {
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

// Registration functions
#ifdef BUILD_PLUGINS
#include <modload.h>
void csnd::on_load(csnd::Csound *csound)
{
    csnd::plugin<BitInv>(csound, "bitinv", "a", "akkkkkkkk", csnd::thread::a);
    csnd::plugin<QuadPan>(csound, "quadpan", "aaaa", "akk", csnd::thread::a);
}
#else
extern "C" int32_t bitinv_init_modules(CSOUND *csound)
{
    csnd::plugin<BitInv>((csnd::Csound *)csound, "bitinv", "a", "akkkkkkkk",
                         csnd::thread::a);
    csnd::plugin<QuadPan>((csnd::Csound *)csound, "quadpan", "aaaa", "akk",
                         csnd::thread::a);
    return OK;
}
#endif