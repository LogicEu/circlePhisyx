#!/bin/bash

src=*.c
cc=gcc
exe=gravity2D

flags=(
    -std=c99
    -Wall
    -Wextra
    -O2
)

inc=(
    -I.
    -Iglee/
    -Ifract/
)

lib=(
    -Llib/
    -lglee
    -lfract
    -lphoton
    -lglfw
)

mac=(
    -framework OpenGL
    #-mmacos-version-min=10.9
)

linux=(
    -lGL
    -lGLEW
)

buildlib() {
    pushd $1 && ./build.sh $2 && mv *.a ../lib/ && popd
}

build() {
    mkdir lib
    buildlib glee -s
    buildlib fract -s
}

comp () {
    if echo "$OSTYPE" | grep -q "darwin"; then
        $cc $src -o $exe ${flags[*]} ${inc[*]} ${lib[*]} ${mac[*]}
    elif echo "$OSTYPE" | grep -q "linux"; then
        $cc $src -o $exe ${flags[*]} ${inc[*]} ${lib[*]} ${linux[*]}
    else 
        echo "OS not supported yet..." && exit
    fi
}

clean() {
    rm -r lib/ && rm $exe
}

fail() {
    echo "$@" && exit
}

case "$1" in
    "-comp")
        comp || fail "Compilation failed"
        echo "Compilation succeded"
        exit;;
    "-run")
        comp || fail "Compilation failed"
        echo "Compilation succeded"
        shift
        ./$exe "$@"
        exit;;
    "-build")
        build || fail "Libraries failed to compile"
        echo "Libraries compiled succesfully"
        exit;;
    "-clean")
        clean || fail "Failed to clean directory"
        echo "Cleaned directory"
        exit;;
    "-all")
        build || fail "Libraries failed to compile"
        echo "Libraries compiled succesfully"
        comp || fail "Compilation failed"
        echo "Compilation succeded"
        shift 
        ./$exe "$@"
        exit;;
    *)
        echo "Use with -comp, -run, -build, -clean, or -all"
        exit;;
esac

