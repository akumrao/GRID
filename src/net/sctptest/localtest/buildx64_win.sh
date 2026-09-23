
rm -rf build
mkdir -p  build/Debug

cp cache.js git add certificate.crt  private_key.pem index.html build/
cp cache.js git add certificate.crt  private_key.pem index.html build/Debug

cmake -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1 -DRTC_STATIC=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
cmake --build build --config Debug

cd ..
