#!/bin/sh

set -xe

build_vc_demo() {
    NAME=$1

    clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--export=init -Wl,--allow-undefined -I. -I./build/ -I./thirdparty/ -o ./build/$NAME.wasm -DPLATFORM=WASM_PLATFORM ./demos/$NAME.c
    mkdir -p ./wasm/
    cp ./build/$NAME.wasm ./wasm/

    clang -Wall -Wextra -ggdb -I. -I./build/ -I./thirdparty/ -o ./build/$NAME.term -DPLATFORM=TERM_PLATFORM ./demos/$NAME.c -lm
}

mkdir -p ./build/

clang -Wall -Wextra -ggdb -o ./build/test -Ithirdparty test.c -lm 
clang -Wall -Wextra -ggdb -o ./build/png2c -Ithirdparty png2c.c -lm 

mkdir -p ./build/assets/
./build/png2c ./assets/nikita.png > ./build/assets/nikita.c

build_vc_demo triangle &
build_vc_demo 3d &
build_vc_demo squish &
wait
