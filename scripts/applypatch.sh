#!/bin/bash

echo "--> Applying patch..."
cd patch && 
find . -type f | while read -r line; do 
  mkdir -vp ../lib/$(dirname "$line" )
  FILE="../lib/$line"
  if [ -f $FILE ]; then
     diff $line "../lib/$line" > /dev/null || cp -vp $line "../lib/$line"
  else
     cp -vp $line "../lib/$line"
  fi
done && cd ..

echo "--> Applying symlinks patched..."
mkdir -p lib/include
rm -vf lib/opusfile/ogg
rm -vf lib/opusfile/opus
ln -vs ../ogg lib/opusfile/ogg
ln -vs ../opus lib/opusfile/opus
ln -vs ../miniupnp/miniupnpc/include lib/include/miniupnpc
