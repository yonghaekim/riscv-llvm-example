######################
#### Need to set #####
######################
CLANG="../_install/bin/clang++"

$CLANG -O3 -march=rv64imac -mabi=lp64 -static \
--target=riscv64-unknown-linux-gnu \
-Wall -Wextra -fPIC -fvisibility=hidden \
-I../sysroot/usr/include \
-B../riscv64-unknown-linux-gnu \
--sysroot=../sysroot \
$1.c -o $1
