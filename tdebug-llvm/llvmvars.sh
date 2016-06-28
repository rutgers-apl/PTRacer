#!/bin/bash
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" #
if [ -z "$PATH" ]; then #
    export PATH="$cwd/build/built/bin" #
else #
    export PATH="$cwd/build/built/bin:$PATH" #
fi #
 #
