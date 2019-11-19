#!/usr/bin/env bash
yarn workspace kungfu-core run clean
yarn clean
sudo npm config set kungfu-core:runtime node
sudo yarn install

cd ./core
yarn run build
cd ../cli
yarn run install 
yarn run install-deps
yarn run build:cli

sudo npm config set kungfu-core:runtime electron
sudo yarn install
cd ../core
yarn run build
cd ../app
yarn run build