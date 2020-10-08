#!/bin/bash

if [ -z ${LLVM_DIR+x} ] || [ -z ${1+x} ];
then
  echo "usage: LLVM_DIR=<dir/llvm/build> $0 <input/file>"
  exit 1
fi

$LLVM_DIR/bin/clang++ -Xclang -disable-O0-optnone -S -emit-llvm -o out.bc $1

$LLVM_DIR/bin/opt -instnamer -mem2reg -break-crit-edges -S -o out.bc out.bc

$LLVM_DIR/bin/opt -load build/libRangeAnalysis.so -load build/libDeadCodeElimination.so -vssa -my-dce -stats -S -o out.bc out.bc