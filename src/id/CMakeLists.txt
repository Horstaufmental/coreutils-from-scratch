option(USE_SELINUX "Build id with SELinux support" OFF)

add_executable(id id.c)
target_compile_definitions(id PRIVATE
    PROGRAM_NAME="id"
)
target_link_libraries(id PRIVATE meta)

if (CACHE{USE_SELINUX})
    message(STATUS "Enabling SELinux support for id")
    target_compile_definitions(id PRIVATE USE_SELINUX)
    target_link_libraries(id PRIVATE selinux)
endif ()
