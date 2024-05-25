
set -e
TOP=$(pwd)
echo "TOP = $TOP"



cd src/openssl

rm -rf openssl/buildx64
rm -rf openssl/openssl
rm -rf openssl/openssl-1.1.1t

if [[ ! -f openssl-1.1.1t.tar.gz ]]; then
    wget 'https://www.openssl.org/source/openssl-1.1.1t.tar.gz'
fi
tar xvf openssl-1.1.1t.tar.gz
mv openssl-1.1.1t openssl
cd openssl

./Configure linux-aarch64 no-shared --prefix="$TOP/src/openssl/buildx64"

make clean
make -j$(nproc)
make -j$(nproc) install

 cd ../../



mkdir -p 3rdparty

echo "Build ffmpeg"
cd 3rdparty

rm -rf ffmpeg
rm -rf ffmpeg-3.4.10

if [[ ! -f ffmpeg-3.4.10.tar.xz ]]; then
    wget 'https://ffmpeg.org/releases/ffmpeg-3.4.10.tar.xz'
fi
tar xvf ffmpeg-3.4.10.tar.xz
mv ffmpeg-3.4.10 ffmpeg
cd ffmpeg


./configure --disable-zlib --disable-msa --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-runtime-cpudetect --disable-lzma --disable-vaapi --prefix="$TOP/3rdparty/installx64"

make -j$(nproc)
make install

cd ../../

