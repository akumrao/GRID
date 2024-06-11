
rm -rf build
mkdir -p build

rm CMakeLists.txt
cp CMakeLists_x64.txt CMakeLists.txt

cd build

cmake -DWEBRTC_REPO=/workspace/MagicAI/src/webrtc/src -DCMAKE_POSITION_INDEPENDENT_CODE=ON  -DANDROID_ABI=x86_64 .. 


make -j$(nproc)

cd ..
