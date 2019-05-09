#!/bin/bash

rm $1 &> /dev/null
rm $1.o &> /dev/null
gcc $1.c -o $1 -Wall &> /dev/null

if [ $? -eq 0 ]
then
    ./$1
else
    echo "COMPILATION ERROR"
fi