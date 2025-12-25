<div align="left">
  <picture>
    <img src="https://github.com/Horstaufmental/Horstaufmental/raw/main/images/coreutils-rs%20from%20scratch.png" width="90%">
  </picture>
</div>

# coreutils-rs from scratch

recreation of GNU's coreutils from scratch (in Rust), solely for learning purposes

## Prologue

i asked ChatGPT what's going to be the best way to learn rust without watching 5 billion tutorials, knowing i currently work on a coreutils clone project it suggested me to do the same but in rust

## Usage

this is mostly just for learning purposes and are not built with the mindset to be used as your daily coreutils programs

### Prerequisites

[Rust](https://rustup.rs) (duh, not the game btw)

### Building

to build all in the workspace:

```bash
    cargo build
```
for a release build:
```bash
    cargo build --release
```

compiled binaries will appear under:
```bash
    target/debug/
    target/release/
```

---

to build only a specific tool (e.g. `cat`):
```bash
    cargo build -p cat
```
run it directly:
```bash
    cargo run -p cat -- --help
```
(note the `--` separating cargo arguments from program arguments)

### Tests

run all tests in the workspace:
```bash
    cargo test
```

run tests for a single tool:
```bash
    cargo test -p cat
```

run tests with output visible:
```bash
    cargo test -p cat -- --nocapture
```

### Supported Platforms

- Linux (primary target)
- macOS (probably)
- Windows (works, but behavior may differ for stdin/paths)

## Features

since this whole purpose are for learning, most utilities will be quite unoptimizied and lacking some functionality from their original GNU's counterpart (and horribly written)

some tools that targets SELinux (e.g. chcon) or one that i deemed unnecessary (e.g. dircolors) are excluded

also these utilities aim to replicate **GNU coreutils behavior**, not provide a rust wrapper API

> **Total of 100 programs: currently 4 has been made**

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
- [ ] ls
- [ ] mkdir
- [ ] mkfifo
- [ ] mknod
- [ ] mktemp
- [ ] mv
- [ ] realpath
- [ ] rm
- [ ] rmdir
- [ ] shred
- [ ] sync
- [ ] touch
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
- [x] head
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
- [ ] arch
- [ ] basename
- [ ] chroot
- [ ] date
- [ ] dirname
- [ ] du
- [ ] echo
- [ ] env
- [ ] expr
- [ ] factor
- [x] false
- [ ] groups
- [ ] hostid
- [ ] id
- [ ] kill
- [ ] link
- [ ] logname
- [ ] nice
- [ ] nohup
- [ ] nproc
- [ ] pathchk
- [ ] pinky
- [ ] printenv
- [ ] printf
- [ ] pwd
- [ ] readlink
- [ ] seq
- [ ] sleep
- [ ] stat
- [ ] stdbuf
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

## License

this project (**coreutils-rs from scratch**) is licensed under the **GNU General Public License v3.0 (GPLv3).**

you are free to:

- use, modify, and redistribute this software under the terms of the GNU GPLv3.
- continue using earlier releases under their original MIT terms

## Information

i practically dont have any other primary source of contact so you can contact me through Discord

![Static Badge](https://img.shields.io/badge/Horstaufmental-%235865f2?style=for-the-badge&label=Discord%20Profile&link=https%3A%2F%2Fdiscord.com%2Fusers%2F880022290023215145)

> dont worry i dont bite
