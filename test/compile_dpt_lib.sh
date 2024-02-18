clang -O3 -march=rv64imac -mabi=lp64 -static \
-Wall -Wextra -fPIC -fvisibility=hidden \
-Wno-deprecated-declarations \
-Wno-unused-command-line-argument \
-Wno-unused-parameter \
-Wno-pointer-to-int-cast \
--target=riscv64-unknown-linux-gnu \
--sysroot=$EX_HOME/_install/sysroot \
-I$EX_HOME/_install/sysroot/usr/include -B$EX_HOME/_install/riscv64-unknown-linux-gnu \
dpt_lib.c -S -emit-llvm -o dpt_lib.ll

sed -i 's/@__tagd(/@llvm.pa.tagd.p0(/' dpt_lib.ll
sed -i 's/@__xtag(/@llvm.pa.xtag.p0(/' dpt_lib.ll
sed -i 's/@__cstr(/@llvm.dpt.cstr(/' dpt_lib.ll
sed -i 's/@__cclr(/@llvm.dpt.cclr(/' dpt_lib.ll
sed -i 's/@__bsetm(/@llvm.bm.bsetm(/' dpt_lib.ll
sed -i 's/@__bclrm(/@llvm.bm.bclrm(/' dpt_lib.ll

clang++ -O3 -march=rv64imac -mabi=lp64 -static \
-Wall -Wextra -fPIC -fvisibility=hidden \
-Wno-deprecated-declarations \
-Wno-unused-command-line-argument \
-Wno-unused-parameter \
-Wno-pointer-to-int-cast \
--target=riscv64-unknown-linux-gnu \
--sysroot=$EX_HOME/_install/sysroot \
-I$EX_HOME/_install/sysroot/usr/include -B$EX_HOME/_install/riscv64-unknown-linux-gnu \
-c dpt_lib.ll -o dpt_lib.o
