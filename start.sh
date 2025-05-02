#/bin/bash
gcc main.c lib.c taixin_sdk.c mongoose.c -o webmodem_test -pthread
./webmodem_test         