#!/bin/sh

set -xe

cc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm
cc -Wall -Wextra -ggdb -o ./bin/gallery -Ithirdparty -I. examples/gallery.c

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--allow-undefined  -o ./bin/triangle.wasm -I. -DPLATFORM=WASM_PLATFORM ./examples/triangle.c
clang -O2 -Wall -Wextra -ggdb -I. -o ./bin/triangle.term -DPLATFORM=TERM_PLATFORM ./examples/triangle.c -lm

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--allow-undefined  -o ./bin/3d.wasm -I. -DPLATFORM=WASM_PLATFORM ./examples/3d.c
clang -O2 -Wall -Wextra -ggdb -I. -o ./bin/3d.term -DPLATFORM=TERM_PLATFORM ./examples/3d.c -lm
