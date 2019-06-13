    mkdir -p resources/default build
    cp -Rf ../core/cpp/wingchun/resources/commission.db ./resources/default
    cp -Rf ../core/cpp/wingchun/resources/holidays.db ./resources/default
    mkdir -p ./build/kfc ./build/resources/default
    cp -Rf ../core/build/kfc/* ./build/kfc 
    cp -Rf ./resources/**/* ./build/resources/default
    cp -Rf ../node_modules/sqlite3/lib/binding/node-v64-darwin-x64/node_sqlite3.node ./build