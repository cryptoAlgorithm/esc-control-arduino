import sys
import threading

import serial

from py.utils import wait_receive

DEFAULT_BAUD = 115200  # Default baud rate that will be used if none is supplied

MAX_SPEED = 1000
BASE_PERIOD = 1000

MOTOR_IDS = ['T', 'B']

# Payloads that we send/expect from controller
STOP_PAYLOAD = 'S'.encode('utf8')
PS_OFF_PAYLOAD = 'P0'.encode('utf8')
READY_ECHO = 'Ready'  # String sent from controller when ready

HEADER = f'''
╔══════════════════════╗
║ ESC Control Frontend ║
║    Version v0.1.1    ║
╚══════════════════════╝
'''.strip()
HELLO_PROMPT = f'''
SAFETY IS OUR PASSION
To immediately stop the motor, hit the enter key at the speed prompt.

Syntax: {','.join([f'<{motor} motor speed>' for motor in MOTOR_IDS])}
Where each motor's speed is [0, 1000]. Leave the speed of a motor blank
to keep it unmodified.

Have fun!
'''
SPEED_PROMPT = f'Set speed ({",".join(MOTOR_IDS)}): '


def ser_setup():
    print('Waiting for controller...')
    print('══════ Controller Init Output (Prefixed with ECHO>): ══════')
    while True:
        line = ser.readline().decode('utf-8')
        print('ECHO>', line, end='')
        if line.strip() == READY_ECHO:
            break
    print('═══════════════════════════════════════════════════════════')
    print(HELLO_PROMPT)


def main_loop():
    while True:
        try:
            raw_in = input(SPEED_PROMPT).strip()
            if len(raw_in) == 0:
                print('Stopping all motors...')
                ser.write(STOP_PAYLOAD)
                continue
            if raw_in == 'o':
                print('Turning off PSU...')
                ser.write(PS_OFF_PAYLOAD)
                continue
            speeds = [None if len(s.strip()) == 0 else int(s.strip()) for s in raw_in.split(',')]
        except ValueError:
            print('Speed is not an integer')
            continue
        # Must have the exact number of RPMs
        if len(speeds) != len(MOTOR_IDS):
            print(f'Wrong number of speeds, expected {len(MOTOR_IDS)} comma-separated numerals but got {len(speeds)}')
            continue

        for idx, speed in enumerate(speeds):
            if speed is None:
                continue
            if not 0 <= speed <= MAX_SPEED:
                print('Speed is out of bounds')
                continue
            motor_id = MOTOR_IDS[idx]
            ser.write((motor_id + str(BASE_PERIOD + speed)).encode('utf-8'))
            # Wait for controller to respond
            print(
                'Setting speed of motor {id:s} to {power:.2f}%... '
                .format(id=motor_id, power=(speed / MAX_SPEED) * 100),
                end=''
            )
            wait_receive(ser, rf'Set interval of ESC <{motor_id}> to \d{{4}}us$')
            print(f'Controller ACK')
        print()


def read_loop():
    while True:
        print(ser.readline(), end='')


read_thread = threading.Thread(target=read_loop)

if __name__ == "__main__":
    print(HEADER)
    if len(sys.argv) < 2:
        print(f'Usage: python {__file__} <port> [baud]')
        exit(2)
    port = sys.argv[1]
    baud = sys.argv[2] if len(sys.argv) >= 3 else DEFAULT_BAUD
    try:
        ser = serial.Serial(port, baud, timeout=10)
    except serial.serialutil.SerialException:
        print(f'FATAL: Failed to open serial port "{port}", check supplied port and permissions.')
        exit(1)
    with ser:
        ser_setup()

        # Uncomment this line to view responses from controller for debugging
        # read_thread.start()

        try:
            main_loop()
        except KeyboardInterrupt:
            print()
            print('Have a nice day!')
            ser.write(STOP_PAYLOAD)
            ser.write(PS_OFF_PAYLOAD)
            exit(0)
        except serial.serialutil.SerialException:
            print()
            print('FATAL: Serial interrupted')
            exit(1)
