cmake_minimum_required(VERSION 3.10)
project(coreutils-from-scratch C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common compiler flags
add_compile_options(-Wall -Wextra -Werror -pedantic -O2 -DNDEBUG -march=native -flto -s)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Single-file utilities
set(SINGLE_FILE_UTILS
    arch
    cat
    echo
    groups
    hostid
    kill
    link
    logname
    mkdir
    mktemp
    nproc
    printenv
    pwd
    rm
    rmdir
    sleep
    touch
    tty
    unlink
    wc
    whoami
    yes
)

# Build each single-file utility
foreach(util ${SINGLE_FILE_UTILS})
    add_executable(${util} src/${util}/${util}.c)
endforeach()

# true and false (from true_false directory)
add_executable(true src/true_false/true.c)
add_executable(false src/true_false/false.c)

# ls - multi-file utility
add_executable(ls
    src/ls/main.c
    src/ls/longformat.c
    src/ls/bytetohr.c
    src/ls/print_help.c
    src/ls/print_version.c
)
target_include_directories(ls PRIVATE src/ls)

# uname - requires OS macro
add_executable(uname src/uname/uname.c)
target_compile_definitions(uname PRIVATE OPERATING_SYSTEM="GNU/Linux")

# id - requires selinux library
add_executable(id src/id/id.c)
target_link_libraries(id selinux)

# Optional: Install targets
install(TARGETS ${SINGLE_FILE_UTILS} true false ls uname id
    RUNTIME DESTINATION bin
)

# Custom target to build all
add_custom_target(all_utils ALL
    DEPENDS ${SINGLE_FILE_UTILS} true false ls uname id
)
