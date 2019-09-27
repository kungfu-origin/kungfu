#!/usr/bin/env bash


yarn clean
cd ./core
npm config set kungfu-core:runtime node
yarn run build:cpp
cd ../cli
yarn run build
cd ../core
yarn clean
npm config set kungfu-core:runtime electron
yarn run build
cd ../app
yarn run build