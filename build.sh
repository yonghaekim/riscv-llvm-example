cmake -G Ninja -DCMAKE_BUILD_TYPE="Release" \
  -DBUILD_SHARED_LIBS=True -DLLVM_USE_SPLIT_DWARF=True \
  -DCMAKE_INSTALL_PREFIX="./_install" \
  -DLLVM_OPTIMIZED_TABLEGEN=True -DLLVM_BUILD_TESTS=False \
  -DDEFAULT_SYSROOT="./_install/riscv64-unknown-linux-gnu" \
  -DLLVM_DEFAULT_TARGET_TRIPLE="riscv64-unknown-linux-gnu" \
	-DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_TARGETS_TO_BUILD="RISCV" \
	-DLLVM_ENABLE_DUMP=ON \
  ./riscv-llvm/llvm

cmake --build . --target install
