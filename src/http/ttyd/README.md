./ttyd -W -I test.html   bash


./ttyd -W -p 8080 bash


 sudo apt install libjson-c-dev




git clone  https://github.com/warmcat/libwebsockets.git (

git checkout v3.2-stable


/workspace/libwebsockets/build$ cmake  -DOPENSSL_ROOT_DIR=/usr/local/google/home/aumrao/workspace/MagicAi/src/openssl/buildx64 -DCMAKE_BUILD_TYPE=Debug -DLWS_WITH_LIBUV=ON ..

sudo make install



git clone https://github.com/tsl0922/ttyd.git

/workspace/ttyd/build$ cmake -DCMAKE_BUILD_TYPE=Debug -DLibwebsockets_DIR=/usr/local/google/home/aumrao/workspace/libwebsockets/build -DCMAKE_PREFIX_PATH=/usr/local/google/home/aumrao/workspace/libwebsockets/ ..


