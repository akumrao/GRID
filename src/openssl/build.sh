#!/bin/bash

chmod +x ./build-openssl.sh


rm -rf openssl


if [[ ! -f openssl-1.1.1t.tar.gz ]]; then
    wget 'https://www.openssl.org/source/openssl-1.1.1t.tar.gz'
fi
tar xvf openssl-1.1.1t.tar.gz
mv openssl-1.1.1t openssl


./build-openssl.sh

