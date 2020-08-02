#!/bin/sh -x

pio run --target upload && pio device monitor | ts "%H:%M:%S" | tee log.txt
