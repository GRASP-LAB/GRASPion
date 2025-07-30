#!/bin/bash

# Number of times to execute
N=20  # Change this to however many times you want

# Your base command without the port
BASE_CMD="arduino-cli compile --fqbn adafruit:samd:adafruit_qtpy_m0 --upload tissue.ino"

# Loop from 0 to N-1
for ((i=0; i<N; i++)); do
    PORT="/dev/ttyACM$i"
    echo "Uploading to $PORT..."
    $BASE_CMD --port "$PORT"
    
    # Optional: wait a bit between uploads
    sleep 1
done
