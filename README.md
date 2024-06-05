# High performing AI for Face Detection for T31 chipset. 


# TO build
sudo bash 

Take ubuntu 18.04 LTS

apt-get install build-essential cmake 

apt-get install  p7zip-full

Install the gcc-9 packages:

sudo apt-get install -y software-properties-common

compilers

apt-get install gcc-9

apt-get install g++-9

apt-get install python


cd /usr/bin

rm gcc

rm  g++ 

ln -s gcc-7 gcc

ln -s g++-7 g++


gcc --version

apt install git

mkdir -p /export/webrtc

cd /export/webrtc

git clone  https://chromium.googlesource.com/chromium/tools/depot_tools 

git checkout chrome/4147


export PATH=/export/webrtc/depot_tools:$PATH 



mkdir -p /workspace/webrtc

cd /workspace/webrtc

fetch --nohooks webrtc_android

cd src

gclient sync

cd src 

git checkout branch-heads/m76

gclient sync -D

mkdir -p /workspace/

T31 compiler path should look like   /workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software  ( shared at google drive)

mkdir /workspace/adappt/T31 cd /workspace/adappt/T31


7z x ISVP-T31-1.1.6-20221229.7z


cd /workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software

chmod +x ./t31_env_setup.sh 

./t31_image_mk.sh


source ./t31_env_setup.sh 540	  repeate again if it says

source ./t31_env_setup.sh 540	



git clone git@github.com:Adappt-Intelligence-Inc/MagicAI.git 

git checkout h264_streaming

cd /workspace/webrtc/src

git apply /workspace/MagicAI/src/webrtc/patch_t31_full.txt


cd /workspace/MagicAI/

./buildt31.sh



For build error 
if you get std::max error because of compiler gcc9  or higer replace

std::max -> std::max<size_t>




# AI
xa_sdk_initialize() 

 I would add xa_sdk_configure()  with the config blob in the Json HOWTO - this will enable face recognition.  A call to  xa_sdk_is_face_recognition_enabled() should return the correct values as specified in the .h file.  Then add a call to  xa_sdk_process_image() with an image with a face you'd like to test - call that function over and over with the same image until it returns some data in process_image_outputs 


#to compile 

cd MagicAI

./build.sh

cd /src/facedetec

make -j8



# to test
copy binary runAI to t31 at /mnt

cd /mnt 
runConfTest 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt




# to test

To compile webrtc


cmake -DWEBRTC_REPO=/workspace/webrtc/src -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DWEBRTC_BUILD_DIR=out/arm64 -DANDROID_ABI=x86_64 .. 
  


cmake -DWEBRTC_REPO=/workspace/webrtc/src -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DWEBRTC_BUILD_DIR=out/arm64 -DANDROID_ABI=mipsel 

cmake -DWEBRTC_REPO=/workspace/webrtc/src -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DWEBRTC_BUILD_DIR=out/arm64 -DCMAKE_CROSSCOMPILING=1  -DANDROID_ABI=mipssel .


install nasm 2.16 from  https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/ . Otherwise include path issues
