#!/usr/bin/env bash
yarn workspace kungfu-core run clean
yarn clean
npm config set kungfu-core:runtime node

cd ./core
yarn run build
cd ../cli
yarn run build-cli

# cd ../
# npm config set kungfu-core:runtime electron
# cd ./core
# yarn run clean
# yarn run build
cd ../app
yarn run build