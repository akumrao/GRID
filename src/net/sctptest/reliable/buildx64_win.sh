
rm -rf build
mkdir -p build

cp cache.js git add certificate.crt  private_key.pem index.html build/


cmake -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1 -DRTC_STATIC=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON 


cd ..
