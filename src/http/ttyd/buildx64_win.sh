
rm -rf build
mkdir -p build

cp index.html build/
cp index.html private_key.pem  certificate.crt  build/

cmake -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -DUSE_MBEDTLS=1  -DCMAKE_POSITION_INDEPENDENT_CODE=ON 


cd ..
