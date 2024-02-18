## Dependencies
You will likely need the following packages.
```
git cmake python-dev libncurses5-dev swig libedit-dev libxml2-dev build-essential gcc-7-plugin-dev clang-6 libclang-6-dev lld-6
```

## Clone Git Repository
```
git clone --recurse-submodules https://github.com/yonghaekim/riscv-llvm-example.git
```

## Build riscv-gnu-toolchain 
```
cd riscv-llvm-example
cd riscv-gnu-toolchain
./configure --prefix=$PWD/../_install --enable-multilib --target=riscv64-linux-gnu
make linux
```

## Build LLVM and Add LLVM path to $PATH
```
./build.sh
export LLVM=$(pwd)
export PATH=$(pwd)/_install/bin:$PATH
```

## Compile a test program
1) First, move to "./test"
```
cd ./test
```

2) Emit the IR file of a test program
```
clang++ -O3 -march=rv64imac -mabi=lp64 -static --target=riscv64-unknown-linux-gnu \
-Wall -Wextra -fPIC -fvisibility=hidden --sysroot=$LLVM/sysroot \
-I$LLVM/sysroot/usr/include -B$LLVM/riscv64-unknown-linux-gnu \
-L$LLVM/riscv64-unknown-linux-gnu \
-S -emit-llvm test.c -o test.ll
```

3) Apply DPT passes to the IR file
```
opt -O0 -dpt-tag -dpt-type=dpt-h -S test.ll -o test_inst.ll
```

4) Compile the instrumented IR file
```
clang++ -O3 -march=rv64imac -mabi=lp64 -static --target=riscv64-unknown-linux-gnu \
-Wall -Wextra -fPIC -fvisibility=hidden --sysroot=$LLVM/sysroot \
-I$LLVM/sysroot/usr/include -B$LLVM/riscv64-unknown-linux-gnu \
-L$LLVM/riscv64-unknown-linux-gnu \
test_inst.ll dpt_lib.o -o test_inst
```

## Original LLVM README
================================

This directory and its subdirectories contain source code for LLVM,
a toolkit for the construction of highly optimized compilers,
optimizers, and runtime environments.

LLVM is open source software. You may freely distribute it under the terms of
the license agreement found in LICENSE.txt.

Please see the documentation provided in docs/ for further
assistance with LLVM, and in particular docs/GettingStarted.rst for getting
started with LLVM and docs/README.txt for an overview of LLVM's
documentation setup.

If you are writing a package for LLVM, see docs/Packaging.rst for our
suggestions.
