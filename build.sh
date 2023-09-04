#!/bin/sh

set -xe

cc -Wall -Wextra -ggdb -o ./bin/example example.c
cc -Wall -Wextra -ggdb -o ./bin/test test.c -lm
