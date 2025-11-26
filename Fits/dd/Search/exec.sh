#! /bin/bash

# Read arguments
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <state> <omp> <start> <stop> <step> <p2 or p3>"
    exit 1
fi

# Read the arguments
state=$1 # state
omp=$2   # omp input dir name
start="${3:-0.24}"
end="${4:-0.30}"
step="${5:-0.01}"
par="${6:-p2}"
echo "-> State: $state and OMP: $omp"
echo "-> Beta: "
echo "    [$start, $end], step = $step"
echo "-> Par : $par"

# Current directory
pwd=$(pwd)

# Make dir
maindir="$pwd/${state}_${omp}"
echo "maindir : ${maindir}"
# Delete it if already exists
if [ -d "$maindir" ]; then
    rm -rf "$maindir"
fi
mkdir -p $maindir

# cd into it
cd $maindir

# Copy file
reference="/media/Data/E796v2/Fits/dd/Inputs/${state}_${omp}/fresco.in"
echo "Reference file: ${reference}"
cp ${reference} .

# Create array of values
beta2s=$(awk -v start="$start" -v end="$end" -v step="$step" 'BEGIN {
    for (i = start; i <= end; i += step) printf "%.3f ", i;
}')

for beta2 in $beta2s; do
    cd $maindir
    echo "Running for beta2 = $beta2"
    # Make dir
    dir="beta_$beta2"
    mkdir -p $dir
    # And now search and replace
    awk -v beta2="$beta2" -v par="$par" '{
    ri = 0
    if (prev ~ /&POT kp=1 type=[12] p\([0-9:]+\)=/){
        split(prev, arr, "p\\([0-9:]+\\)=");
        split(arr[2], vals, " ");
        # Clean up extra whitespace before using vals
        for (i = 1; i <= length(vals); i++) {
            vals[i] = gensub(/^[ \t]+|[ \t]+$/, "", "g", vals[i]);  # Trim leading/trailing whitespace
        }
        ri = vals[2] * 20^(1.0/3)
    }
    if ($0 ~ "&POT kp=1 type=11 " par "=") {
        count++
        if (count == 2 || count == 3 || count == 4) {
            sub(par"=[^ ]*", par "=" ri * beta2)
        }
    }
    prev = $0
    print
    }' fresco.in >temp_file && mv temp_file $dir/fresco.in
    # cd and execute fresco
    cd $dir
    fresco <fresco.in >fresco.out
    # leave only interesting files
    find . -maxdepth 1 ! -name 'fort.20*' ! -name 'fresco.in' ! -name '*.out' -type f -exec rm -f {} \;
done
