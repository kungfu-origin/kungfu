#!/usr/bin/env bash

yarn clean
cd ./core
npm config set kungfu-core:runtime node
yarn run build
cd ../cli
yarn run build
# cd ../core
# pwd
# yarn clean
# npm config set kungfu-core:runtime electron
# yarn run build
cd ../app
pwd
yarn run build