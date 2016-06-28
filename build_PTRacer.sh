#!/bin/bash

root_cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" #
echo $root_cwd

echo "***** Building LLVM *****"
cd tdebug-llvm
make
source llvmvars.sh

echo "***** Building PTRacer *****"
export LD_LIBRARY_PATH=""
cd $root_cwd
cd tdebug-lib
make clean
make
source tdvars.sh

echo "***** Building TBB *****"
cd $root_cwd
cd tbb-lib
make clean
make
source obj/tbbvars.sh

echo "***** Building PTSolver *****"
cd $root_cwd
cd PTRacer-solver
source solvervars.sh
cd $root_cwd
