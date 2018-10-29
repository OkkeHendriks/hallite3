#!/usr/bin/env bash

set -e

mkdir -vp build

cd build && cmake ../
make -j
cd ..

exec ./halite --replay-directory replays/ -vvv --width 32 --height 32 "build/MyBot" "build/MyBot"
