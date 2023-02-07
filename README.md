# Arduino ESC Controller

Some firmware for an Arduino Nano to act as a controller for 2 ESCs.

The firmware provides a 115200-baud serial interface is designed to be
used with `esc-control.py`, but can also be used with a plain serial
terminal.

This code should support any Arduino-compatible board that's supported
by the Servo library. To build the project for your board, update the
`platform =` and `board =` entries in `platformio.ini`.