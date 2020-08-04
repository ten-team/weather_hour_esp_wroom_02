#!/bin/sh -x

PWD_ABSPATH=$(pwd)
LOG_FILE=${PWD_ABSPATH}/log.txt

DIR_NAME=`dirname $0`
cd ${DIR_NAME}/..
pio run --target upload && pio device monitor | ts "%H:%M:%S" | tee ${LOG_FILE}
cd -
