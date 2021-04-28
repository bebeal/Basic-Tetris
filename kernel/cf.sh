#!/bin/bash

F32=`gcc -m32 --print-file-name $1`
F64=`gcc -m32 --print-file-name $1`

if test "$F32" == "$F64"; then
    M=~gheith/public/$1
    if test -f $M; then
        echo $M
    else
        echo $F32
    fi
else
    echo $F32
fi
