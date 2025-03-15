# HVCC External Generator for logue SDK V1

This project is an external generator for [HVCC](https://github.com/Wasted-Audio/hvcc). It generates code and other necessary files for the KORG [logue SDK](https://github.com/korginc/logue-sdk) v1.1 from a Pure Data patch. Currently, only the oscillator unit for SDK v1 is supported (for Prologue, Minilogue XD, and NTS-1).

## Installation

Clone this repository. In addition, you must have HVCC and the logue SDK installed.

## Usage

1. From the repository's directory, run:

   ```bash
   hvcc YOUR_PUREDATA_PATCH.pd -G loguesdk_v1 -n PATCH_NAME
   ```
   In case your `loguesdk_v1` directory is not in `PYTHONPATH` then add it by:
   ```bash
   export PYTHONPATH=$PYTHONPATH:$(pwd)
   ```

2. Move the directory named `logue_unit` into one of the logue SDK platform directories (e.g., `logue-sdk/platform/prologue`, `logue-sdk/platform/minilogue-xd`, or `logue-sdk/platform/nutekt-digital`).

3. In the `logue_unit` directory, run:

   ```bash
   make install
   ```

   You can also specify your platform without placing your project directory under `logue-sdk/platform` as compile option like:
   
   ```
   make PLATFORMDIR="~/logue-sdk/platform/nutekt-digital" install
   ```

**Please note:**

- The `[dac~]` object in your Pure Data patch must be single-channel (i.e., `[dac~ 1]`).
- The `[adc~]` object generates no sound.

## Interacting with the logue SDK API

### Knobs

- The `[r shape @hv_param]` object receives the shape knob value as an integer (0 to 1023). Alternatively, `[r shape_f @hv_param]` receives a floating-point value between 0.0 and 1.0.
- The `[r alt @hv_param]` object receives the shift-shape knob value as an integer (0 to 1023). Alternatively, `[r alt_f @hv_param]` receives a floating-point value between 0.0 and 1.0.

### Pitch and LFO

- The `[r pitch @hv_param]` object receives the oscillator pitch frequency in Hz. Alternatively, the `[r pitch_note @hv_param]` object receives the oscillator pitch as a floating-point note number.
- The `[r slfo @hv_param]` object receives the shape LFO value, which ranges from -1.0 to 1.0. (NTS-1's LFO generates values between 0.0 and 1.0.) Note that the shape LFO is a control value, not a signal value.

### Note Events

- The `[r noteon_trig @hv_param]` object receives a `bang` when a MIDI Note On event occurs.
- The `[r noteoff_trig @hv_param]` object receives a `bang` when a MIDI Note Off event occurs.

### Parameters

Any `[r]` object whose variable name does not match the ones described above but includes the `@hv_param` parameter is recognized as an oscillator parameter. Up to six oscillator parameters can be used.

### Specifying parameter slot number

Optionally, you can specify the parameter slot by adding a prefix `_N_` (where N is the parameter slot number from 1 to 6) to the variable name. The string without the prefix is used as the variable name on the synthesizer's display. For example, the variable name `_3_ratio` assigns the parameter "ratio" to slot 3.

### Receiving floating-point values

By default, all variables receive raw integer values from the logue SDK API. You can specify minimum and maximum values, which must be between -100 and 100.

A variable name with the postfix `_f` receives floating-point values between 0.0 and 1.0 (mapped from integer values between 0 and 100). You can optionally specify the minimum, maximum, and default values using the syntax:

```
[r varname @hv_param min max default]
```

In this case, the floating-point values are mapped from integer values between -100 and 100.

### Parameter type

Currently all parameters are defined as percentage type because typeless parameters are not allowed to be  negative values, and the displayed values are different between NTS-1 and prologue/minilogue xd.

## Restrictions

### Supported Unit Type

Only oscillator-type units are supported. Other logue SDK user unit types (such as mod, delay, or reverb) are not supported because they do not have enough memory space to run an HVCC context.

### Memory Footprint

The oscillator unit must fit within a 32,767-byte space. All necessary resources (including code, constants, variables, and heap/stack) must be in this space. You'll got a linker error when the binary size exceeds this boundary.

### DAC

The logue SDK oscillator units support only a single-channel DAC with a 48,000 Hz sampling rate.

## Appendix

### Size of the heap memory

Due to the 32KB momory space limitation, the heap size of an oscillator unit must be determined before compiling & building it. The heap size can be specified as a compiler flag like this:

```makefile
-DUNIT_HEAP_SIZE=3072
```

To estimate required heap memory size, this external generator generates a C source file `testmem.c` and a Makefile `Makefile.testmem`. This code is built and called from `project.mk` and the result is stored into `logue_heap_size.mk` .

You can check the `malloc()` calls and total amount of requested memory space for generating the first 6400 samples like this:

```bash
make -f Makefile.testmem
./testmem
```
### Math functions approximation

Some of the math functions have been replaced with the logue SDK functions which finds the approximate value. If you got inaccurate results, comment out the line 
```
UDEFS += -DLOGUE_FAST_MATH
```
to disable replacement. Note that in general, this will result in a larger binary size.
