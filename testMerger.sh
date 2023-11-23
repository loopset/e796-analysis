#!/bin/bash

for i in {1..3}
do
  echo "========================================================"
  echo "Repetition $i"
  cmd="root -l -q -x DoMerge.cxx"
  $cmd
  status=$?
  if [ $status -eq 1 ]
  then
      echo "Error running ROOT!"
      exit 1
  fi
done
