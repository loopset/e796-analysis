#!/bin/bash

# Get which file
file=$1
echo "Using fresco file: ${file}"

# Get output dir
key=$2
echo "Writing in key dir: ${key}"

current_dir=$(pwd)
echo "In dir: ${current_dir}"

full_file="${current_dir}/${file}"

# Define values of the real potential
rs=(1.1 1.15 1.2 1.25 1.28 1.3 1.33 1.35 1.4 1.45 1.5 1.55)
# Scaling factor between real and SO potential
# 1.1 / 1.25
scale=$(awk 'BEGIN{printf "%.4f\n", (1.10/1.25)}')

for r in "${rs[@]}"; do
    dirname="./Inputs/${key}/r_ws_${r}/"
    path="${current_dir}/${dirname}"
    echo "path : ${path}"
    mkdir -p ${path}
    cd ${path}
    # Replace value of r0 in fresco.in
    rso=$(awk -v scale="$scale" -v r="$r" 'BEGIN{printf "%.4f\n", (scale * r)}')
    echo "Run for r0: ${r} and rso = $rso"
    awk -v r0="$r" -v rso="$rso" '
    {
    if ($2 == "kp=4" && $3 == "type=1") $6 = r0
    else if ($2 == "kp=4" && $3 == "type=3") $6 = rso
    print
    }' "$full_file" >fresco.in
    # Run fresco in that dir
    fresco <fresco.in> fresco.out
done
