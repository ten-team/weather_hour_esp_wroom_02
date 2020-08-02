#!/bin/sh -x

LOG_FILE=log_unix.txt
LOG4GNUPLOT_FILE=log4gnuplot.txt

cat ${LOG_FILE} | awk 'BEGIN{FS="[, ]"} $6=="Free" {print $1 " " $5 " " $9}' > ${LOG4GNUPLOT_FILE}

OUTFILE="${LOG4GNUPLOT_FILE%.*}.png"
TITLE="Monitor memory"
XLABEL="Time"
YLABEL="Line"
Y2LABEL="Memory"

gnuplot <<EOF
set xdata time
set timefmt '%Y/%m/%d %H:%M:%S'
set format x '%H:%M:%S'

set y2tics
set title '$TITLE'
set term png size 800,480
set xlabel '$XLABEL'
set ylabel '$YLABEL'
set y2label '$Y2LABEL'
set output '$OUTFILE'
p '$LOG4GNUPLOT_FILE' u 0:2 w l ti 'line','$LOG4GNUPLOT_FILE' u 0:3 w l axes x1y2 ti 'memory'
set output
EOF
