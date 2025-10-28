<div align="left">
  <picture>
    <img src="https://github.com/Horstaufmental/Horstaufmental/raw/main/images/coreutils-from-scratch%20full%20icon.png" width="90%">
  </picture>
</div>

# coreutils-from-scratch

recreation of GNU's coreutils from scratch, solely for learning purposes

## Prologue

one day at school, as i was about to hop on tf2 to play during a lunch break, an idea suddenly came to my mind:

> What if I recreate the coreutils all from scratch?

so that's how this project came to be

## Usage

this is mostly just for learning purposes and are not built with the mindset to be used as your daily coreutils programs

for most tools that only consisted of one single source code file, `.c` can be simply complied with a compiler in your terminal like so:

```
$ gcc mkdir.c -o mkdir
```

while tools that have more than one files will usually include a `Makefile`, which can be simply be built by running `make` in your terminal

a prebuilt binary (usually for x86_64 devices) can be found in most of the tools (usually in the each of the respective tool's folder or inside their `bin` folder)

## Features

since this whole purpose are for learning, most utilities will be quite unoptimizied and lacking some functionality from their original GNU's counterpart (and horribly written)

some tools that targets SELinux (e.g. chcon) or one that i deemed unnecessary (e.g. dircolors) are excluded

> **Total of 100 programs: currently 24 has been made**

### File Utilities (fileutils) List
- [ ] chgrp
- [ ] chown
- [ ] chmod
- [ ] cp
- [ ] dd
- [ ] df
- [ ] install
- [ ] join
- [ ] ln
- [x] ls
- [x] mkdir
- [ ] mkfifo
- [ ] mknod
- [x] mktemp
- [ ] mv
- [ ] realpath
- [x] rm
- [x] rmdir
- [ ] shred
- [ ] sync
- [x] touch
- [ ] vdir
### Text Utilities (textutils) List
- [ ] b2sum
- [ ] base32
- [ ] base64
- [ ] basenc
- [x] cat
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
- [ ] sha1sum
- [ ] sha224sum
- [ ] sha256sum
- [ ] sha384sum
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
- [x] arch
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
- [x] groups
- [x] hostid
- [x] id
- [x] kill
- [ ] link
- [ ] logname
- [ ] nice
- [ ] nohup
- [x] nproc
- [ ] pathchk
- [ ] pinky
- [x] printenv
- [ ] printf
- [x] pwd
- [ ] readlink
- [ ] seq
- [x] sleep
- [ ] stat
- [ ] stdbuf
- [ ] tee
- [ ] test
- [ ] timeout
- [x] true
- [x] tty
- [x] uname
- [x] unlink
- [ ] uptime
- [ ] users
- [ ] who
- [x] whoami
- [x] yes

## License

originally this project is licensed under the **MIT License**, but starting from version 1.0 (aka Okami Era), is now licensed under the **GNU General Public License v3.0 (GPLv3).**

(all versions without versions/file headers prior to this change are not considered part of the new GPLv3-licensed releases)

you are free to:

- use, modify, and redistribute this software under the terms of the GNU GPLv3.
- continue using earlier releases under their original MIT terms

## Information

i practically dont have any other primary source of contact so you can contact me through Discord

![Static Badge](https://img.shields.io/badge/Horstaufmental-%235865f2?style=for-the-badge&label=Discord%20Profile&link=https%3A%2F%2Fdiscord.com%2Fusers%2F880022290023215145)

> dont worry i dont bite
