#!/bin/bash

dist=$1

# Cd to config dir
cd /media/Data/E796v2/configs/

# Set file to modify
file="simu_silicons.conf"

# Keep gap between f0 and f1 constant!!
gap=14.5 ## pad units

# Reminder: dist is in PAD UNITS

awk -v dist="$dist" -v gap="$gap" '{
if ($0 ~ /Point:/){
    count++
    if(count == 1){ # first occurrence == f0
        sub(/Point: [^,]+/, "Point: " dist)
    } else if(count == 2){
        distplusgap = dist + gap
        sub(/Point: [^,]+/, "Point: " distplusgap)
    }

}
print
}' $file >temp_file && mv temp_file $file
