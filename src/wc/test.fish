#!/usr/bin/fish

gcc -O3 -march=native -pipe -flto -DNDEBUG -pedantic -o wc wc.c && echo Finished Compiling! && strip wc && echo Executable stripped!, running benchmark... && time ./wc -cmlLw --total=auto ./gibberish-utf8-ascii-mix.txt
