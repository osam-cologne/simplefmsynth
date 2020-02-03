#!/bin/bash

exec env LD_LIBRARY_PATH=$(pwd)/gamma/build/lib \
LV2_PATH=$(pwd)/bin \
jalv.gtk "$@" https://chrisarndt.de/plugins/simplefmsynth
