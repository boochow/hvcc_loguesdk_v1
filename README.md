# hvcc External Generator for logue SDK V1

This project is an external generator for [hvcc](https://github.com/Wasted-Audio/hvcc). It generates code and other necessary files for the KORG [logue SDK](https://github.com/korginc/logue-sdk) from a Pure Data patch. Currently, only the oscillator unit for SDK v1.1 is supported (targeting Prologue, Minilogue XD, and NTS-1).

## Installation

Clone this repository. In addition, ensure that both hvcc and the logue SDK are installed. You also need GCC/G++ to estimate the required heap memory size; see the Appendix for details.

## Usage

1. Add the `loguesdk_v1` directory to your `PYTHONPATH` by running:

   ```bash
   export PYTHONPATH=$PYTHONPATH:path-to-hvcc_loguesdk_v1
   ```
 
   Then, run:

   ```bash
   hvcc YOUR_PUREDATA_PATCH.pd -G loguesdk_v1 -n PATCH_NAME -o DESTINATION_DIR
   ```
 
   Check the directory `DESTINATION_DIR`; it should contain four directories named `c`, `hv`, `ir`, and `logue_unit`.

2. Move the directory named `logue_unit` into one of the supported logue SDK platform directories (e.g., `logue-sdk/platform/prologue`, `logue-sdk/platform/minilogue-xd`, or `logue-sdk/platform/nutekt-digital`).

3. In the `logue_unit` directory, run:

   ```bash
   make install
   ```
 
   Alternatively, you can specify your platform via a compile-time option without moving your project directory under `logue-sdk/platform`:

   ```bash
   make PLATFORMDIR="~/logue-sdk/platform/nutekt-digital" install
   ```

**Notes about your Pure Data patch:**

- The `[dac~]` object in your Pure Data patch must be single-channel (i.e., `[dac~ 1]`).
- The `[adc~]` object does not receive sound.
- For a list of objects supported by hvcc, refer [here](https://github.com/Wasted-Audio/hvcc/blob/develop/docs/09.supported_vanilla_objects.md).

## Examples

A separate repository containing sample patches for this project is available at:

[https://github.com/boochow/loguesdk_hvcc_examples](https://github.com/boochow/loguesdk_hvcc_examples)

## Receiving Parameters in Your Pure Data Patch

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

By default, all variables receive raw integer values from the logue SDK API. You can specify a minimum value (between -100 and 100) and a maximum value (between 0 and 100).

A variable with the postfix `_f` receives a floating-point value between 0.0 and 1.0 (mapped from integer values between 0 and 100). You can optionally specify the minimum, maximum, and default values using the syntax:

```
[r varname @hv_param min max default]
```

In this case, the floating-point values are mapped from integer values between -100 and 100.

#### Parameter Type

Currently, all parameters are defined as "percentage type" because typeless parameters cannot have negative values, and the values shown on the display differ between the NTS-1 and Prologue/Minilogue XD.

## Restrictions

### Supported Unit Type

Only oscillator-type units are supported. Other logue SDK user unit types (such as mod, delay, or reverb) are not supported because they do not have enough memory space to run an hvcc context.

### Memory Footprint

The oscillator unit must fit within a 32,767-byte space. All necessary resources—including code, constants, variables, and both heap and stack—must reside within this space. A linker error will occur if the binary size exceeds this boundary.

### DAC

The logue SDK oscillator units support only a single-channel DAC with a 48,000 Hz sampling rate.

## Appendix

### Size of the Heap Memory

Due to the 32KB memory space limitation, you must specify the heap size of an oscillator unit before the build process. The heap size can be set as a compiler flag:

```makefile
-DUNIT_HEAP_SIZE=3072
```

The heap size estimation process is integrated into `project.mk`, or you can manually specify the size like this:

```bash
make HEAP_SIZE=4096
```

For automatic estimation, this external generator creates `testmem.c` and `Makefile.testmem`. When GCC and G++ are available, `testmem.c` is built and executed from `project.mk`, and the estimated heap size is saved in `logue_heap_size.mk`.

You can check the `malloc()` calls and the total requested memory for generating the first 6400 samples by running:

```bash
make -f Makefile.testmem
./testmem
```

If GCC and G++ are not available in your development environment, the default heap size (3072 bytes) is used. (Note: In the Docker image version of the logue SDK build environment, GCC/G++ are not provided, so the default heap size is always used.)

### Math Functions Approximation

Some math functions have been replaced with logue SDK functions that provide approximate values. If you get inaccurate results, comment out the following line in `project.mk`:

```makefile
UDEFS += -DLOGUE_FAST_MATH
```

to disable the fast math approximation. Note that disabling this may result in a larger binary size.

### Internal Sampling Rate

To reduce processing load, an oscillator unit generated by this generator calculates only half of the requested sample frames and interpolates them to generate the other half. This results in the sound lacking harmonics above 12 KHz, and many artifacts may appear in higher notes if no band-limiting technique is used. You can edit the `project.mk` file and comment out the line:

```makefile
UDEFS += -DRENDER_HALF
```

to have the oscillator calculate all sample frames. However, this requires much more processing power and may cause the oscillator unit to become unstable for complex Pure Data patches.

### Filling a Table with White Noise

Any table whose name ends with `_r` that is exposed using the `@hv_table` notation is always filled with white noise using the logue SDK function `osc_white()`. This feature can be used to replace the `[noise~]` object with the much lighter `[tabread~]` object.

However, note that:

1. Filling the buffer with `osc_white()` requires significant processing time, so keep the table size as small as possible (typically 16 or 32 samples).
2. To generate white noise, you also need a `[phasor~ freq]`, `[*~ tablesize]`, and `[tabread~]` object. The value of `freq` should be `48000 / tablesize`.

## Credits

* [Heavy Compiler Collection (hvcc)](https://github.com/Wasted-Audio/hvcc) by Enzien Audio and maintained by Wasted Audio
* [logue SDK](https://github.com/korginc/logue-sdk) by KORG
* [Pure Data](https://puredata.info/) by Miller Puckette
