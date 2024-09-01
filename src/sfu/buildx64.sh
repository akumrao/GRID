rm -rf build
mkdir -p build

cd build

cmake -DCROSS_PLATEFORM=x86_64 ..

make -j$(nproc)

