# How to run:

Inside of the pass folder run:

```
export LLVM_DIR=<installation/dir/of/llvm>
mkdir build
cd build
CXX=$LLVM_DIR/bin/clang++ cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ..
make
```

The pass is compiled out of the LLVM tree.