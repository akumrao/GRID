
rm -rf build
mkdir -p build

cp cache.js certificate.crt  private_key.pem index.html build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCROSS_PLATEFORM=x86_64 -DENABLE_OPENSSL=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
