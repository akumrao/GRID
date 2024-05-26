
rm -rf build
mkdir -p build

rm CMakeLists.txt
cp CMakeLists_t31.txt CMakeLists.txt

cd build

cmake -DWEBRTC_REPO=/workspace/webrtc/src -DCMAKE_POSITION_INDEPENDENT_CODE=ON  -DWEBRTC_BUILD_DIR=out/arm64 -DANDROID_ABI=aarch64 ..


make -j$(nproc)

cd ..





