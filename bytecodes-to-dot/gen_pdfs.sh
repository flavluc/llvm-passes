#!/bin/bash

for i in `ls inputs`; do clang "inputs/${i}" -S -emit-llvm -o "inputs/${i}.bc"; done

for i in `find inputs -name *.bc`; do mkdir "${i}.dir" && opt -load-pass-plugin build/libByteCodeToDot.so -passes=bc-to-dot "${i}" -disable-output && mv *.dot "${i}.dir"; done

for i in `find inputs -name *.dir`; do for j in `find $i -name *.dot`; do dot -Tpdf $j > "${j}.pdf"; done; done