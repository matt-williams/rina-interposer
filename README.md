# RINA Interposer

Prototype adaptor from BSD Sockets/POSIX API calls to RINA

To build:

    gcc -shared -fPIC interposer.c -ldl /usr/lib/librina-api.so -o libinterposer.so

On server, run:

    RINA_DIF=normal.DIF RINA_LOCAL_APPL=nc-server LD_PRELOAD=$(pwd)/libinterposer.so nc -l 1.2.3.4 1234

On client, run:

    RINA_DIF=normal.DIF RINA_LOCAL_APPL=nc-client RINA_REMOTE_APPL=nc-server LD_PRELOAD=$(pwd)/libinterposer.so nc 1.2.3.4 1234

For more detail, set the `RINA_VERBOSE` (to anything).
