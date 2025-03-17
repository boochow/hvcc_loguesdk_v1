# HVCC External Generator for logue SDK V1

This project is an external generator for [HVCC](https://github.com/Wasted-Audio/hvcc). It generates code and other necessary files for the KORG [logue SDK](https://github.com/korginc/logue-sdk) v1.1 from a Pure Data patch. Currently, only the oscillator unit for SDK v1 is supported (targeting Prologue, Minilogue XD, and NTS-1).

## Installation

Clone this repository. In addition, ensure that both HVCC and the logue SDK are installed. You also need gcc/g++ to estimate the required heap memory size; see the Appendix for details.

## Usage

1. Add the `loguesdk_v1` directory to your `PYTHONPATH` by:

   ```bash
   export PYTHONPATH=$PYTHONPATH:path-to-hvcc_loguesdk_v1
   ```

   Then, run:

   ```bash
   hvcc YOUR_PUREDATA_PATCH.pd -G loguesdk_v1 -n PATCH_NAME -o DESTINATION_DIR
   ```

   Check the directory `DESTINATION_DIR`. There should be 4 directories named `c`, `hv`, `ir`, and `logue_unit`.

2. Move the directory named `logue_unit` into one of the logue SDK platform directories (e.g., `logue-sdk/platform/prologue`, `logue-sdk/platform/minilogue-xd`, or `logue-sdk/platform/nutekt-digital`).

3. In the `logue_unit` directory, run:

   ```bash
   make install
   ```

   Alternatively, you can specify your platform via a compile-time option without moving your project directory under `logue-sdk/platform`:

   ```bash
   make PLATFORMDIR="~/logue-sdk/platform/nutekt-digital" install
   ```

**Please note:**

- The `[dac~]` object in your Pure Data patch must be single-channel (i.e., `[dac~ 1]`).
- The `[adc~]` object does not receive sound.
- For a list of objects supported by HVCC, refer [here](https://github.com/Wasted-Audio/hvcc/blob/develop/docs/09.supported_vanilla_objects.md).

## Receiving parameters in your Pure Data patch

### Knobs

- The `[r shape @hv_param]` object receives the shape knob value as an integer (0 to 1023). Alternatively, `[r shape_f @hv_param]` receives a floating-point value between 0.0 and 1.0.
- The `[r alt @hv_param]` object receives the shift-shape knob value as an integer (0 to 1023). Alternatively, `[r alt_f @hv_param]` receives a floating-point value between 0.0 and 1.0.

### Pitch and LFO

- The `[r pitch @hv_param]` object receives the oscillator pitch frequency in Hz. Alternatively, `[r pitch_note @hv_param]` receives the oscillator pitch as a floating-point note number.
- The `[r slfo @hv_param]` object receives the shape LFO value, which ranges from -1.0 to 1.0 (note that the NTS-1’s LFO generates values between 0.0 and 1.0). Remember that the shape LFO is a control value, not a signal value.

### Note Events

- The `[r noteon_trig @hv_param]` object receives a `bang` when a MIDI Note On event occurs.
- The `[r noteoff_trig @hv_param]` object receives a `bang` when a MIDI Note Off event occurs.

### Parameters

Any `[r]` object whose variable name does not match those described above but includes the `@hv_param` parameter is recognized as an oscillator parameter. Up to six oscillator parameters can be used.

#### Specifying Parameter Slot Number

Optionally, you can specify the parameter slot by adding a prefix `_N_` (where N is a number from 1 to 6) to the variable name. The remainder of the name is used as the variable name on the synthesizer’s display. For example, the variable name `_3_ratio` assigns the parameter "ratio" to slot 3.

#### Receiving Floating-Point Values

By default, all variables receive raw integer values from the logue SDK API. You can specify minimum and maximum values, which must be between -100 and 100.

A variable with the postfix `_f` receives a floating-point value between 0.0 and 1.0 (mapped from integer values between 0 and 100). You can optionally specify the minimum, maximum, and default values using the syntax:

```
[r varname @hv_param min max default]
```

In this case, the floating-point values are mapped from integer values between -100 and 100.

#### Parameter Type

Currently, all parameters are defined as percentage type because parameters without a specified type cannot have negative values, and the displayed values differ between the NTS-1 and Prologue/Minilogue XD.

## Restrictions

### Supported Unit Type

Only oscillator-type units are supported. Other logue SDK user unit types (such as mod, delay, or reverb) are not supported because they do not have enough memory space to run an HVCC context.

### Memory Footprint

The oscillator unit must fit within a 32,767-byte space. All necessary resources—including code, constants, variables, and both heap and stack—must reside within this space. A linker error will occur if the binary size exceeds this boundary.

### DAC

The logue SDK oscillator units support only a single-channel DAC with a 48,000 Hz sampling rate.

## Appendix

### Size of the Heap Memory

Due to the 32KB memory space limitation, you must specify the heap size of an oscillator unit during the build process. The heap size can be set as a compiler flag, for example:

```makefile
-DUNIT_HEAP_SIZE=3072
```

The heap size is automatically estimated in `project.mk`, or you can manually specify the size like this:

```bash
make HEAP_SIZE=4096
```

To estimate the required heap memory size, this external generator creates a C source file (`testmem.c`) and a Makefile (`Makefile.testmem`). 

When gcc and g++ are available in your environment, this code is built and executed from `project.mk`, and the estimated heap size is saved in `logue_heap_size.mk`.

You can check the `malloc()` calls and the total requested memory for generating the first 6400 samples by running:

```bash
make -f Makefile.testmem
./testmem
```

If gcc and g++ are not available in your development environment, the default heap size (3072 bytes) is used. (Note: In the Docker image version of the logue SDK build environment, gcc/g++ are not provided, so the heap size will always default.)

### Math Functions Approximation

Some math functions have been replaced with logue SDK functions that provide approximate values. If you get inaccurate results, comment out the line:

```makefile
UDEFS += -DLOGUE_FAST_MATH
```

to disable the fast math approximation. Note that disabling this may result in a larger binary size.

### Internal Sampling Rate

To reduce processing load, this generator sets the sampling rate of an oscillator unit to 24 KHz by default. This is half the actual oscillator rate, so the sound will lack harmonics above 12 KHz. You can edit the `project.mk` file and comment out the line:

```makefile
UDEFS += -DRENDER_HALF
```

to run the oscillator at a 48 KHz sampling rate. However, this may cause the synthesizer to behave abnormally for complex pure data patches.
