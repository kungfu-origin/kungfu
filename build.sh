#!/usr/bin/env bash

yarn clean
sudo npm config set kungfu-core:runtime node
sudo yarn install

cd ./core
yarn run build
cd ../cli
yarn run build

sudo npm config set kungfu-core:runtime electron
sudo yarn install
cd ../core
yarn run build
cd ../app
yarn run build