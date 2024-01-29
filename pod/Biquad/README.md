# Biquad

## Author

Alexander Wu

## Description

First, clone [DaisyExamples](https://github.com/electro-smith/DaisyExamples), and follow the instructions to build the libraries. Then, place this folder in a directory two levels below `DaisyExamples` (such as in `DaisyExample/pod`). Open this folder in VS Code, then in the command palette (command+P), run `task build_and_program_dfu` (or run `make` and `make program-dfu` in the command line).

| Control | Description |
|--|--|
| Button 1 | Trigger white noise |
| Knob 1 | Center frequency (20–20k Hz) |
| Knob 2 | Q (0.00001–100) |
| Encoder (Rotate) | Gain (-80–12 Hz) |
| Encoder (Press) | Cycle through different filter types |

Encoder (Rotate) has not been implemented because gain is only used for shelf and peak filters, which are not yet supported.

Available filters:
- low pass
- high pass
- band pass