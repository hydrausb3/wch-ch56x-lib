#!/bin/sh

find . \( -iname  "*.h" -o -iname "*.c" \) -print0 | xargs -0 clang-format --verbose --style=file -i;
