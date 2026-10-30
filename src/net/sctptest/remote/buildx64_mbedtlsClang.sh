
rm -rf buildmbed
mkdir -p buildmbed

cp cache.js git add certificate.crt  private_key.pem index.html buildmbed
cd buildmbed

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1  -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_CXX_FLAGS="-m64" -DCMAKE_C_FLAGS="-m64" .. 

#cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DUSE_MBEDTLS=1   -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
