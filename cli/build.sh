#!/usr/bin/env bash
if [ "$(uname)" == "Darwin" ]; then
    npm run build:mac
    cp -RF ./resources/mac/* ./build/cli 
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    npm run build:linux
    cp -RF ./resources/linux/* ./build/cli 
fi