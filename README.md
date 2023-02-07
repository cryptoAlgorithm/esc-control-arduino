# Arduino ESC Controller

Some firmware for an Arduino Nano to act as a controller for 2 ESCs.

The firmware provides a 115200-baud serial interface is designed to be
used with [the provided frontend](#frontend), but can also be used with
a plain serial terminal.

This code should support any Arduino-compatible board that's supported
by the Servo library. To build the project for your board, update the
`platform =` and `board =` entries in `platformio.ini`.

## Frontend

A Python frontend is also provided in the [`py`](py) folder for ease
of interacting with the serial interface.

### Running the Frontend

1. Activate the venv in [`venv`](venv)
2. Run the main Python script:
   ```bash
   python3 py/esc_control.py <port>
   ```