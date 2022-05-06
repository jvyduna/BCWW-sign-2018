#! /bin/sh

set -e

tmp_dir=$(mktemp -d -t virtual-sign)
echo cd $tmp_dir

for f in ../sign_control_vProd/*{ino,h} *.cpp *.h; do
  if [ -f $f ]; then cp $f $tmp_dir; fi
done

cd $tmp_dir
for file in *.ino; do
   mv -i "$file" "${file%.ino}.cpp"
done

for f in *.cpp *.h
do
  if [ x$f != xvirtual_sign.h -a -f $f ]; then
    perl -pli -e 'if ($.== 1) { print q[#include "virtual_sign.h"] }' $f
  fi
done


for f in *.cpp; do
  if [ -f $f ]; then
    g++ -c -g -O0 -Wall -cxx-isystem . $f
  fi
done

g++ -lcurses *.o -o virtual_sign

./virtual_sign 2> /tmp/virtual_sign.err
