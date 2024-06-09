
set -e
TOP=$(pwd)
echo "TOP = $TOP"









# no-shared no-dso no-makedepend no-aria no-async no-autoload-config no-blake2 no-bf no-camellia no-cast no-chacha no-cmac no-cms no-comp no-ct no-des no-dgram no-dh no-dsa no-dtls no-ec2m no-engine no-filenames no-gost no-idea no-mdc2 no-md4 no-multiblock no-nextprotoneg no-ocsp no-ocb no-poly1305 no-psk no-rc2 no-rc4 no-rmd160 no-seed no-siphash no-sm2 no-sm3 no-sm4 no-srp no-srtp no-ssl3 no-ssl3-method no-ts no-ui-console no-whirlpool no-asm -DOPENSSL_SMALL_FOOTPRINT
#workspace/MagicAI/src/openssl/buildt31 no-asan no-async no-buildtest-c++ no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-heartbeats no-md2 no-msan no-rc5 no-sctp no-shared no-ssl-trace no-ssl3 no-ssl3-method no-ubsan no-unit-test no-weak-ssl-ciphers no-zlib no-zlib-dynamic

#no-cmac

# no-shared no-afalgeng no-async no-capieng  no-cms no-comp no-ct \
#  no-dgram no-dso no-dynamic-engine no-engine no-filenames no-gost \
# no-nextprotoneg no-ocsp   no-sock \
# no-ssl-trace no-static-engine no-tests  no-ts \
# no-ui-console \
# no-bf no-blake2  \
# no-dsa no-idea no-md4 no-mdc2 no-ocb no-rc2 no-rc4 no-rmd160 no-scrypt \
# no-asan no-async no-buildtest-c++ no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-heartbeats no-md2 no-msan no-rc5 no-sctp no-shared no-ssl-trace no-ubsan no-unit-test no-weak-ssl-ciphers no-zlib no-zlib-dynamic \
# no-shared no-dso no-makedepend no-aria no-async no-autoload-config no-blake2 no-bf no-camellia no-cast no-chacha  no-cms no-comp no-ct no-des no-dgram no-dh no-dsa no-ec2m no-engine no-filenames no-gost no-idea no-mdc2 no-md4 no-multiblock no-nextprotoneg no-ocsp no-ocb no-poly1305 no-psk no-rc2 no-rc4 no-rmd160 no-seed no-siphash no-sm2 no-sm3 no-sm4 no-srp  no-ts no-ui-console no-whirlpool no-asm -DOPENSSL_SMALL_FOOTPRINT \
# no-siphash  no-sm2 no-sm3 no-sm4 no-whirlpool --prefix="$TOP/src/openssl/buildx64"



#./Configure no-shared no-asan no-async no-buildtest-c++ no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-heartbeats no-md2 no-msan no-rc5 no-sctp no-shared no-ssl-trace no-ssl3 no-ssl3-method no-ubsan no-unit-test no-weak-ssl-ciphers no-zlib no-zlib-dynamic --prefix="$TOP/src/openssl/buildx64"





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

./Configure linux-x86_64 no-shared no-shared no-asan no-async no-buildtest-c++ no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-heartbeats no-md2 no-msan no-rc5 no-sctp no-shared no-ssl-trace no-ssl3 no-ssl3-method no-ubsan no-unit-test no-weak-ssl-ciphers no-zlib no-zlib-dynamic \
no-afalgeng no-async no-capieng  no-cms no-comp no-ct \
no-ui-console \
no-bf no-blake2 no-scrypt \
no-ssl-trace no-static-engine no-tests no-ts  no-buildtest-c++ no-crypto-mdebug \
no-dso no-makedepend no-aria no-async no-autoload-config no-blake2 no-bf no-camellia no-cast no-chacha  no-comp no-ct  no-dh no-ec2m no-engine no-filenames no-gost no-idea no-mdc2 no-md4 no-multiblock no-nextprotoneg no-ocsp no-ocb no-poly1305 no-psk no-rc2 no-rc4 no-rmd160 no-seed no-siphash no-sm2 no-sm3 no-sm4 no-srp  no-ts no-ui-console no-whirlpool no-asm -DOPENSSL_SMALL_FOOTPRINT \
no-siphash  no-sm2 no-sm3 no-sm4 no-whirlpool --prefix="$TOP/src/openssl/buildx64"
# no-dsa no-idea no-md4 no-mdc2      \
# no-asan no-async no-buildtest-c++ no-crypto-mdebug   no-dso no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd  no-ubsan     \
# no-dso  no-comp no-ct no-des no-dgram no-dh no-dsa no-ec2m no-engine no-filenames no-gost no-idea 

# no-dgram    \ // it create problem

#no-sock \ // it create problem


make clean
make -j$(nproc)
make -j$(nproc) install

 cd ../../



# mkdir -p 3rdparty

# echo "Build ffmpeg"
# cd 3rdparty

# rm -rf ffmpeg
# rm -rf ffmpeg-3.4.10

# if [[ ! -f ffmpeg-3.4.10.tar.xz ]]; then
#     wget 'https://ffmpeg.org/releases/ffmpeg-3.4.10.tar.xz'
# fi
# tar xvf ffmpeg-3.4.10.tar.xz
# mv ffmpeg-3.4.10 ffmpeg
# cd ffmpeg


# ./configure --disable-zlib --disable-msa --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-runtime-cpudetect --disable-lzma --disable-vaapi --prefix="$TOP/3rdparty/installx64"

# make -j$(nproc)
# make install

# cd ../../

