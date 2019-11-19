#!/usr/bin/env bash
yarn workspace kungfu-core run clean
yarn clean
npm config set kungfu-core:runtime node
yarn install

cd ./core
yarn run build
cd ../cli
yarn run install 
yarn run install-deps
yarn run build:cli

cd ../
npm config set kungfu-core:runtime electron
yarn install
cd .core
yarn run build
cd ../app
yarn run build