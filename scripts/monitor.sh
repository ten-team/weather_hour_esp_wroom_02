#!/bin/sh -x

DIR_NAME=basename `pwd`
LOG_FILE=${DIR_NAME}/log.txt

cd ..
pio run --target upload && pio device monitor | ts "%H:%M:%S" | tee ${LOG_FILE}
cd -
