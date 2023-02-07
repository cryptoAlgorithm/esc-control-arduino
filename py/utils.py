import re

from serial import Serial

PROMPT = 'Set interval (1000-2000us): '


def wait_ack(ser: Serial, pattern: str, regex: bool = False, limit: int = None):
    lines = 0
    has_limit = limit is not None
    limit = limit if has_limit else 1
    while lines < limit:
        line = ser.readline().decode('utf-8').strip().replace(PROMPT, '')
        # print(f'Receive: "{line}"')
        if regex:
            if re.search(pattern, line) is not None:
                break
        else:
            if line == pattern:
                break
        if has_limit:
            lines += 1
