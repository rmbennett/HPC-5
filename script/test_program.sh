#!/bin/bash

echo "Generating output"
cat script/input512.raw | bin/process 512 512 2 2>/dev/null > script/output512.bin
cat script/input1024.raw | bin/process 1024 1024 2 2>/dev/null > script/output1024.bin

echo "Verifying results"

echo " "

if diff script/output512.bin script/original512.bin >/dev/null ; then
  if diff script/output1024.bin script/original1024.bin >/dev/null ; then
  	echo "######################################"
    echo "######### Files Identical :) #########"
    echo "######################################"
  else
  	echo "################################"
    echo "######### 1024 Differs #########"
    echo "################################"
  fi
else
  echo "###############################"
  echo "######### 512 Differs #########"
  echo "###############################"
fi

echo " "

echo "Cleaning up coz I'm a mucky, mucky pup..."
rm script/output1024.bin script/output512.bin