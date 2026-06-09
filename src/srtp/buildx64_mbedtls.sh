
rm -rf buildmbed
mkdir -p buildmbed

cp cache.js certificate.crt  private_key.pem index.html buildmbed
cd buildmbed

cmake -DCMAKE_BUILD_TYPE=Debug -DCROSS_PLATEFORM=x86_64 -DUSE_MBEDTLS=1  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

#cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DUSE_MBEDTLS=1   -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
