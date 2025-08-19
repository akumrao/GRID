
rm -rf build
mkdir -p build


cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DWEBRTC_REPO=/workspace/MagicAI/src/webrtcserver  -DWORKSPACE=/workspace/MagicAI  -DCMAKE_POSITION_INDEPENDENT_CODE=ON  .. 


make -j$(nproc)

cd ..
