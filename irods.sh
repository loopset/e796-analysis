#!/bin/bash

home="/ccin2p3/home/actartpc/E796/ROOT_2022_03/"

icd $home

echo "iRODS at : ${home}"

# {Alpha calibration at end}
for run in {418..418}; do
  echo "$run"
  formatted=$(printf "%04d" "$run")
  iget -V "Tree_Run_${formatted}_Merged.root" "./RootFiles/Raw/"
done

