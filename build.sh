#!/usr/bin/env bash

yarn clean
npm config set kungfu-core:runtime node
yarn install

cd ./core
yarn run build
cd ../cli
yarn run build


# npm config set kungfu-core:runtime electron
# yarn install
# cd ../core
# yarn run build
# cd ../app
# yarn run build