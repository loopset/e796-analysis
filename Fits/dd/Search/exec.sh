#! /bin/bash

# Read arguments
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <state> <omp> <start> <stop> <step>"
    exit 1
fi

# Read the arguments
state=$1 # state
omp=$2   # omp input dir name
start="${3:-0.24}"
end="${4:-0.30}"
step="${5:-0.01}"
echo "-> State: $state and OMP: $omp"
echo "-> Beta2: "
echo "    [$start, $end], step = $step"

# Set radius of nucleus
R=$(awk 'BEGIN {print 1.3 * 20^(1.0/3)}')
echo "    radius: $R"

# Current directory
pwd=$(pwd)

# Make dir
maindir="$pwd/$state"
# Delete it if already exists
if [ -d "$maindir" ]; then
    rm -rf "$maindir"
fi
mkdir -p $maindir

# cd into it
cd $maindir

# Copy file
cp "../../Inputs/${state}_${omp}/fresco.in" .

# Create array of values
beta2s=$(awk -v start="$start" -v end="$end" -v step="$step" 'BEGIN {
    for (i = start; i <= end; i += step) printf "%.3f ", i;
}')

for beta2 in $beta2s; do
    cd $maindir
    delta2=$(awk -v b="$beta2" -v r="$R" 'BEGIN {printf "%.3f", b * r}')
    echo "Running for delta2 = $delta2"
    # Make dir
    dir="beta_$beta2"
    mkdir -p $dir
    # And now search and replace
    awk -v delta2="$delta2" '{
    if ($0 ~ /&POT kp=1 type=11 p2=/) {
        count++
        if (count == 2 || count == 3) {
            sub(/p2=[^ ]*/, "p2=" delta2)
        }
    }
    print}' fresco.in >temp_file && mv temp_file $dir/fresco.in
    # cd and execute fresco
    cd $dir
    fresco <fresco.in >fresco.out
    # leave only interesting files
    find . -maxdepth 1 ! -name 'fort.20*' ! -name 'fresco.in' ! -name '*.out' -type f -exec rm -f {} \;

done
