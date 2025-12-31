#!/bin/sh

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

info()  { echo -e "${CYAN}-- $1${NC}"; }
doneok()  { echo -e "${GREEN}✔ $1${NC}"; }
error() { echo -e "${RED}✖ $1${NC}"; }

cd external/tracy/

echo -e "$CYAN- CMake tracy... $NC"
cmake -B profiler/build -S profiler -DCMAKE_BUILD_TYPE=Release -DLEGACY=ON > /dev/null &&
echo -e "$GREEN- CMake tracy Done! $NC"
cd profiler/build
echo -e "$CYAN- Make tracy... $NC"
make -j4 > /dev/null &&
echo -e "$GREEN- Make tracy Done! $NC"
cd ../..
cd ../..
rm -f tracy-profiler
ln -s ./external/tracy/profiler/build/tracy-profiler tracy-profiler
