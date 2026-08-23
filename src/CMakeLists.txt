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
        head
)

set(SPECIAL_CASE_UTILS
        ls
        true_false
        basenc
        id
        uname
)

# Build each single-file utility
foreach (util ${SINGLE_FILE_UTILS})
    add_executable(${util} ${util}/${util}.c)
    target_link_libraries(${util} PRIVATE meta)
    target_compile_definitions(${util} PRIVATE
        PROGRAM_NAME="${util}"
    )
endforeach ()

foreach (dir ${SPECIAL_CASE_UTILS})
    add_subdirectory(${dir})
endforeach ()

# Optional: Install targets
install(TARGETS ${SINGLE_FILE_UTILS} true false ls uname basenc id
        RUNTIME DESTINATION bin
)

# Custom target to build all
add_custom_target(all_utils ALL
        DEPENDS ${SINGLE_FILE_UTILS} true false ls uname basenc id
)
