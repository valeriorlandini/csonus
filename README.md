# csonus
Custom CSound operators for creative sound design and algorithmic composition.

Currently implemented opcodes are:

- `bitinv`
    - Purpose: Invert selected bits of an 8-bit quantized audio signal.
    - Usage: `a bitinv in, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8`
    - Inputs: audio input + eight boolean (0/1) controls for bit inversion (LSB = bit1, MSB = bit8).
    - Behavior: Clips input to [-1, 1], quantizes to 8 bits (0–255), XORs selected bits, then rescales to [-1, 1].

- `lorenz`
    - Purpose: Generates three audio-rate signals corresponding to the x/y/z coordinates of a Lorenz attractor.
    - Usage: ax, ay, az lorenz amp, speed, beta, sigma, rho
    - Inputs: amplitude, speed, beta, sigma, rho
    - Behavior: Steps a Lorenz system each sample and outputs scaled/clamped x, y, z suitable as modulation sources or audio textures.

- `perceptron`
    - Purpose: Simple step-function threshold operator (classical perceptron behavior).
    - Usage: `a perceptron in, weight, bias`
    - Inputs: audio input, weight, bias
    - Behavior: Computes weight*in + bias and outputs 1.0 if > 0, else 0.0.

- `phasedist`
    - Purpose: Phase-distortion oscillator (classic PD technique).
    - Usage: `a phasedist amp, freq, d`
    - Inputs: amplitude, frequency, distortion parameter d
    - Behavior: Runs a PDOsc oscillator from the soutel utilities; set sample-rate and parameters for each block.

- `pulsar`
    - Purpose: Pulsar oscillator (short bursts derived from microsound techniques).
    - Usage: `a pulsar amp, freq, duty_cycle, waveform, window`
    - Inputs: amplitude, frequency, duty cycle, waveform selector, window selector
    - Behavior: Uses a Pulsar implementation from soutel; supports selectable waveform and window shapes.

- `quadpan`
    - Purpose: Quadraphonic panning of a single input signal to four outputs (FL, FR, RL, RR).
    - Usage: `afl, afr, arl, arr quadpan in, x, y`
    - Inputs: audio input, x (left-right 0..1), y (front-rear 0..1)
    - Behavior: Distributes input across four outputs using cosine-based amplitude law for smooth panning.

- `roessler`
    - Purpose: Generates three audio-rate signals corresponding to the x/y/z coordinates of a Roessler attractor.
    - Usage: `ax, ay, az roessler amp, speed, a, b, c`
    - Inputs: amplitude, speed, a, b, c
    - Behavior: Steps a Roessler system each sample and outputs scaled/clamped x, y, z suitable as modulation sources or audio textures.


License & Attribution
---------------------
This collection is distributed under the MIT License. See the LICENSE file for the full text and copyright attribution.