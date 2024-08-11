rm -rf build
mkdir -p build

cd build

cmake -DUSE_MUCLIBC=ON -DBUILD_TESTING=off -DANDROID_ABI=x86_64 ..

make -j$(nproc)


cp -r ../frames/ ../config.js  ../build/