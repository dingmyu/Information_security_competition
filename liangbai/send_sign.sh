#!bin/sh
gcc -o get_digest get_digest.c sm3.h sm3.c 
./get_digest
gcc -o sendA  sendA.c sm2.c kdf.h sm2.h libcrypto.a libssl.a -L /usr/local/ssl/include -I /usr/local/ssl/lib -ldl
./sendA

