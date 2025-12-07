
rm -rf build
mkdir -p build

cp index.html build/

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

#cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DUSE_MBEDTLS=1   -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
