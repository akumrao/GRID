
rm -rf buildmbed
mkdir -p buildmbed

cp cache.js buildmbed
cd buildmbed

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

#cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DUSE_MBEDTLS=1   -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
