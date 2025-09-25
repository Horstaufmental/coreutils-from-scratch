# coreutils-from-scratch

recreation of GNU's coreutils from scratch, primarily for learning purposes

## Prologue

one day at school, as i was about to hop on tf2 to play, an idea suddenly came to my mind:

> What if I recreate the coreutils all from scratch?

so that's how this repo came to be

## Usage

unless if you wanted to test the tools, there's no point of using these tools for daily use. just use actual well maintained implementations like from GNU or BusyBox.

for most tools that only consisted of one single source code file, `.c` can be simply complied with a compiler in your terminal like so:

```
$ gcc mkdir.c -o mkdir
```

while tools that have more than one files will usually include a `Makefile`, which can be simply be built by running `make` in your terminal

a prebuilt binary (for x86_64 devices) can be found in most of the tools (usually in the folder `bin` in each respective tool's folder)

## Features

since this whole purpose are for learning, most utilities will be quite unoptimizied and lacking most functionality from their original GNU's counterpart (and horribly written)

### File Utilities (fileutils) List
- [ ] chgrp
- [ ] chown
- [ ] chmod
- [ ] cp
- [ ] dd
- [ ] df
- [ ] install
- [ ] ln
- [x] ls
- [x] mkdir
- [ ] mkfifo
- [ ] mknod
- [ ] mktemp
- [ ] mv
- [ ] realpath
- [x] rm
- [ ] rmdir
- [ ] shred
- [ ] sync
- [ ] touch
### Text Utilities (textutils) List
- [ ] b2sum
- [ ] base32
- [ ] base64
- [ ] basenc
- [ ] cat
- [ ] cksum
- [ ] comm
- [ ] csplit
- [ ] cut
- [ ] expand
- [ ] fmt
- [ ] fold
- [ ] head
- [ ] md5sum
- [ ] nl
- [ ] numfmt
- [ ] od
- [ ] paste
- [ ] ptx
- [ ] pr
- [ ] sha512sum
- [ ] shuf
- [ ] sort
- [ ] split
- [ ] sum
- [ ] tac
- [ ] tail
- [ ] tr
- [ ] tsort
- [ ] unexpand
- [ ] uniq
- [ ] wc
### Shell Utilities (shellutils) List
- [ ] arch
- [ ] basename
- [ ] chroot
- [ ] date
- [ ] dirname
- [ ] du
- [x] echo
- [ ] env
- [ ] expr
- [ ] factor
- [x] false
- [ ] groups
- [ ] hostid
- [ ] id
- [ ] link
- [ ] logname
- [ ] nice
- [ ] nohup
- [x] nproc
- [ ] pathchk
- [ ] printenv
- [ ] printf
- [x] pwd
- [ ] readlink
- [ ] seq
- [ ] sleep
- [ ] stat
- [ ] tee
- [ ] test
- [ ] timeout
- [x] true
- [ ] tty
- [ ] uname
- [ ] unlink
- [ ] uptime
- [ ] users
- [ ] who
- [ ] whoami
- [ ] yes

## Information

i practically dont have any other primary source of contact so you can contact me through Discord

![Static Badge](https://img.shields.io/badge/Horstaufmental-%235865f2?style=for-the-badge&label=Discord%20Profile&link=https%3A%2F%2Fdiscord.com%2Fusers%2F880022290023215145)

> dont worry i dont bite
