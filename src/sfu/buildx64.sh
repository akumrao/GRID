rm -rf build
mkdir -p build

cd build

cmake  -DANDROID_ABI=x86_64 ..

make -j$(nproc)

