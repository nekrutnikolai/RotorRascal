#!/usr/bin/env python3

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import re

# Serial config
PORT = '/dev/tty.usbmodem13101'  # Change this!
BAUDRATE = 115200
MAX_POINTS = 250

# Setup regex
pattern = re.compile(r"wx:(.*?), wy:(.*?), wz:(.*?), wxf:(.*?), wyf:(.*?), wzf:(.*?), ax:(.*?), ay:(.*?), az:(.*?), axf:(.*?), ayf:(.*?), azf:(.*?)\s*$")

# Initialize data storage
fields = ['wx', 'wy', 'wz', 'wxf', 'wyf', 'wzf', 'ax', 'ay', 'az', 'axf', 'ayf', 'azf']
data = {key: deque([0.0]*MAX_POINTS, maxlen=MAX_POINTS) for key in fields}
t = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)

# Setup plot
fig, ax = plt.subplots()
lines = {key: ax.plot([], [], label=key)[0] for key in fields}
ax.set_xlim(0, MAX_POINTS)
ax.set_ylim(-10, 10)
ax.legend(loc='upper left')
ax.set_title("IMU Live Stream")

# Open serial port
ser = serial.Serial(PORT, BAUDRATE, timeout=1)

def update(frame):
    global t
    try:
        line = ser.readline().decode().strip()
        match = pattern.match(line)
        if match:
            values = list(map(float, match.groups()))
            for key, val in zip(fields, values):
                data[key].append(val)
            t.append(t[-1] + 1)
            for key in fields:
                lines[key].set_data(range(len(t)), data[key])
            ax.set_xlim(max(0, len(t) - MAX_POINTS), len(t))
    except Exception as e:
        print("Parse error:", e)
    return list(lines.values())

ani = animation.FuncAnimation(fig, update, interval=50)
plt.show()