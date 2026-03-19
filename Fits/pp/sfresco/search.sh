#!/bin/bash

# --- Input ranges ---
start0="${1:-0.5}"
end0="${2:-1.5}"
step0="${3:-0.05}"
start1="${4:-0.5}"
end1="${5:-1.5}"
step1="${6:-0.05}"

ref="fresco.in"
[ ! -f "$ref" ] && {
  echo "fresco.in not found"
  exit 1
}

# --- Generate arrays ---
vals0=()
val="$start0"
while (($(echo "$val <= $end0" | bc -l))); do
  vals0+=("$val")
  val=$(echo "$val+$step0" | bc -l)
done
vals1=()
val="$start1"
while (($(echo "$val <= $end1" | bc -l))); do
  vals1+=("$val")
  val=$(echo "$val+$step1" | bc -l)
done

for cte0 in "${vals0[@]}"; do
  for cte1 in "${vals1[@]}"; do
    dir="./Outputs/cte_$(echo $cte0 | sed 's/\./p/')_$(echo $cte1 | sed 's/\./p/')"
    mkdir -p "$dir"

    awk -v cte0="$cte0" -v cte1="$cte1" '
    {
      line=$0
      if ($0 ~ /&POT kp=1 type=1 /) {
        if (match(line,/p1=[^ ]+/)){v=substr(line,RSTART+3,RLENGTH-3); sub(/p1=[^ ]+/,"p1="v*cte0,line)}
        if (match(line,/p4=[^ ]+/)){v=substr(line,RSTART+3,RLENGTH-3); sub(/p4=[^ ]+/,"p4="v*cte1,line)}
      }
      if ($0 ~ /&POT kp=1 type=2 /) {
        if (match(line,/p4=[^ ]+/)){v=substr(line,RSTART+3,RLENGTH-3); sub(/p4=[^ ]+/,"p4="v*cte1,line)}
      }
      print line
    }' "$ref" >"$dir/fresco.in"

    (cd "$dir" && command -v fresco >/dev/null 2>&1 && fresco <fresco.in >fresco.out &&
      find . -maxdepth 1 -type f ! -name 'fresco.in' ! -name 'fresco.out' ! -name 'fort.201*' -exec rm -f {} \;)
  done
done
