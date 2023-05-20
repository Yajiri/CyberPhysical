set title "Group 02"
set xlabel "timeStamp"
set ylabel "GroundSteering"
plot "GS.txt" title "GS" w l lc "#0a0a0a", "calculateGS.txt" title "computedGS" w l lc "#ff0015"
