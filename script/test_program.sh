#!/bin/bash

echo "Verifying 512_d1"
if ! diff <(cat script/input512_d1.raw | bin/oldprocess 512 512 1 1 2>/dev/null | hexdump) <(cat script/input512_d1.raw | bin/process 512 512 1 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d1 l1 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d1.raw | bin/oldprocess 512 512 1 2 2>/dev/null | hexdump) <(cat script/input512_d1.raw | bin/process 512 512 1 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d1 l2 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d1.raw | bin/oldprocess 512 512 1 3 2>/dev/null | hexdump) <(cat script/input512_d1.raw | bin/process 512 512 1 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d1 l3 Differs #########"
  echo "#####################################"
  exit -1
fi

echo "Verifying 512_d2"
if ! diff <(cat script/input512.raw | bin/oldprocess 512 512 2 1 2>/dev/null | hexdump) <(cat script/input512.raw | bin/process 512 512 2 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d2 l1 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512.raw | bin/oldprocess 512 512 2 -1 2>/dev/null | hexdump) <(cat script/input512.raw | bin/process 512 512 2 -1 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d2 l-1 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512.raw | bin/oldprocess 512 512 2 2 2>/dev/null | hexdump) <(cat script/input512.raw | bin/process 512 512 2 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d2 l2 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512.raw | bin/oldprocess 512 512 2 -2 2>/dev/null | hexdump) <(cat script/input512.raw | bin/process 512 512 2 -2 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d2 l-2 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512.raw | bin/oldprocess 512 512 2 3 2>/dev/null | hexdump) <(cat script/input512.raw | bin/process 512 512 2 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d2 l3 Differs #########"
  echo "#####################################"
  exit -1
fi

echo "Verifying 512_d4"
if ! diff <(cat script/input512_d4.raw | bin/oldprocess 512 512 4 1 2>/dev/null | hexdump) <(cat script/input512_d4.raw | bin/process 512 512 4 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d4 l1 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d4.raw | bin/oldprocess 512 512 4 2 2>/dev/null | hexdump) <(cat script/input512_d4.raw | bin/process 512 512 4 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d4 l2 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d4.raw | bin/oldprocess 512 512 4 3 2>/dev/null | hexdump) <(cat script/input512_d4.raw | bin/process 512 512 4 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d4 l3 Differs #########"
  echo "#####################################"
  exit -1
fi

echo "Verifying 512_d8"
if ! diff <(cat script/input512_d8.raw | bin/oldprocess 512 512 8 1 2>/dev/null | hexdump) <(cat script/input512_d8.raw | bin/process 512 512 8 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d8 l1 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d8.raw | bin/oldprocess 512 512 8 2 2>/dev/null | hexdump) <(cat script/input512_d8.raw | bin/process 512 512 8 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d8 l2 Differs #########"
  echo "#####################################"
  exit -1
fi

if ! diff <(cat script/input512_d8.raw | bin/oldprocess 512 512 8 3 2>/dev/null | hexdump) <(cat script/input512_d8.raw | bin/process 512 512 8 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "#####################################"
  echo "######### 512_d8 l3 Differs #########"
  echo "#####################################"
  exit -1
fi

echo "Verifying 512_d16"
if ! diff <(cat script/input512_d16.raw | bin/oldprocess 512 512 16 1 2>/dev/null | hexdump) <(cat script/input512_d16.raw | bin/process 512 512 16 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d16 l1 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512_d16.raw | bin/oldprocess 512 512 16 2 2>/dev/null | hexdump) <(cat script/input512_d16.raw | bin/process 512 512 16 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d16 l2 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512_d16.raw | bin/oldprocess 512 512 16 3 2>/dev/null | hexdump) <(cat script/input512_d16.raw | bin/process 512 512 16 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d16 l3 Differs #########"
  echo "######################################"
  exit -1
fi

echo "Verifying 512_d32"
if ! diff <(cat script/input512_d32.raw | bin/oldprocess 512 512 32 1 2>/dev/null | hexdump) <(cat script/input512_d32.raw | bin/process 512 512 32 1 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d32 l1 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512_d32.raw | bin/oldprocess 512 512 32 2 2>/dev/null | hexdump) <(cat script/input512_d32.raw | bin/process 512 512 32 2 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d32 l2 Differs #########"
  echo "######################################"
  exit -1
fi

if ! diff <(cat script/input512_d32.raw | bin/oldprocess 512 512 32 3 2>/dev/null | hexdump) <(cat script/input512_d32.raw | bin/process 512 512 32 3 2>/dev/null | hexdump) >/dev/null ; then
  echo "######################################"
  echo "######### 512_d32 l3 Differs #########"
  echo "######################################"
  exit -1
fi

echo "Verifying 1024_d2"
for i in {1..64..8}
do
  echo "Image 1024 Depth 2 Level $i"
  if ! diff <(cat script/input1024.raw | bin/oldprocess 1024 1024 2 $i 2>/dev/null) <(cat script/input1024.raw | bin/process 1024 1024 2 $i 2>/dev/null) >/dev/null ; then
    echo "######################################"
    echo "######### 1024_d2 l$i Differs #########"
    echo "######################################"
    exit -1
  fi
done

echo " "

echo "######################################"
echo "######### Files Identical :) #########"
echo "######################################"

echo " "