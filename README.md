# RINA Interposer

Prototype adaptor from BSD Sockets/POSIX API calls to RINA

To build:

    gcc -shared -fPIC interposer.c -ldl rlite/build/user/libs/librina-api.so -o libinterposer.so

To run:

    RINA_DIF=normal.DIF RINA_LOCAL_APPL=nc-local RINA_REMOTE_APPL=nc-remote LD_PRELOAD=$(pwd)/libinterposer.so nc 1.2.3.4 1234
