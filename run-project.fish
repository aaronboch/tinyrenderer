#!/usr/bin/env fish

set obj (find . -name '*.obj' | fzf --prompt='Select .obj > ')

if test -n "$obj"
    ./build/release/tinyrenderer "$obj"
end
