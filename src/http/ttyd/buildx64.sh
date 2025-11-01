
rm -rf build
mkdir -p build

cp index.html build/

cd build

#cmake  -DCMAKE_BUILD_TYPE=Debug -DWORKSPACE=/usr/local/google/home/aumrao/workspace/MagicAi  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

cmake -DCMAKE_BUILD_TYPE=Debug -DWORKSPACE=/workspace/MagicAI  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 

make -j$(nproc)

cd ..
