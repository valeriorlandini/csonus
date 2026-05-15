# csonus
Custom CSound operators for creative sound design and algorithmic composition.

Currently implemented opcodes are:

- `bitinv`
    - Purpose: Invert selected bits of an 8-bit quantized audio signal.
    - Usage: `a bitinv in, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8`
    - Inputs: audio input + eight boolean (0/1) controls for bit inversion (LSB = bit1, MSB = bit8).
    - Behavior: Clips input to [-1, 1], quantizes to 8 bits (0–255), XORs selected bits, then rescales to [-1, 1].
    
- `byteplay`
    - Purpose: Classic bytebeat waveform generator using user-selected formula and sample-rate reduction.
    - Usage: `a byteplay amp, formula_idx, sr_red`
    - Inputs: amplitude, formula index (0–15), sample-rate reduction factor
    - Behavior: Produces a single evolving audio signal from a bytebeat expression; formula_idx selects the algorithm, and sr_red controls the effective playback rate and evolution speed.

- `cheby`
    - Purpose: Distortion using a Chebyshev polynomial applied to the input signal.
    - Usage: `a cheby in, order`
    - Inputs: audio input, polynomial order (0–10; fractional values interpolate between nearest integer orders).
    - Behavior: Processes the input through a Chebyshev function of the specified order, producing increasingly complex harmonic distortion as the order increases.

- `cryptoverb`
    - Purpose: Eerie stereo reverberation effects with three block-processing modes.
    - Usage: `al, ar cryptoverb in_l, in_r, wet, mode, lowcut`
    - Inputs: stereo audio input, wet mix level, mode selector (0, 1, 2), lowcut frequency.
    - Behavior: Processes the signal through a configurable series of allpass and comb filters; the selected mode changes the filter arrangement and reverb character, while lowcut controls overall brightness.

- `genetic`
    - Purpose: A genetic algorithm that evolves a population of strings toward a target string.
    - Usage: `S genetic target, dictionary, population_size, mutation_rate, mating_rate, multiple_crossover`
    - Inputs: target string, allowed character dictionary, population size, mutation rate, mating rate, multiple crossover flag (optional, defaults to 0).
    - Behavior: Starts with a random population of strings and repeatedly applies selection, crossover, and mutation to converge toward the target. The best matching string is output at each control, evolving over time as the algorithm progresses.

- `linden`
    - Purpose: A configurable Lindenmayer system generator.
    - Usage: `S linden axiom, rules, max_length`
    - Inputs: axiom (initial sequence string), rules (production rules string), maximum output length (optional, defaults to no truncation).
    - Behavior: Takes a string and a set of production rules (formatted as "A>AB,B>AC,C>BAA"), and applies the rules iteratively to generate a new sequence at each control, outputting the evolved string. The maximum string length parameter prevents excessively long outputs as the system evolves, keeping the last part of the string.

- `lorenz`
    - Purpose: Generates three audio-rate signals corresponding to the x/y/z coordinates of a Lorenz attractor.
    - Usage: ax, ay, az lorenz amp, speed, beta, sigma, rho
    - Inputs: amplitude, speed, beta, sigma, rho
    - Behavior: Steps a Lorenz system each sample and outputs scaled/clamped x, y, z suitable as modulation sources or audio textures.
    
- `neurosc`
    - Purpose: Neural waveform generator with user-controlled latent weights.
    - Usage: `a neurosc amp, freq, w1, w2, w3, w4, w5, w6, w7, w8, window`
    - Inputs: amplitude, frequency, eight latent weights, window flag
    - Behavior: Generates a single audio signal from a neural network autoencoder with eight latent variables, enabling a wide range of timbres and smoothly evolving shapes.

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

- `tent`
    - Purpose: Control-rate tent map generator for chaotic modulation.
    - Usage: `k tent start, mu`
    - Inputs: initial value (0.0–1.0), mu parameter (0.0–2.0; values above ~1.41 produce chaos)
    - Behavior: Scales and biases the incoming control value, then applies a piecewise linear tent map to create complex, evolving control output.


License & Attribution
---------------------
This collection is distributed under the MIT License. See the LICENSE file for the full text and copyright attribution.