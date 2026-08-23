add_executable(uname uname.c)
if (LINUX)
  set(OPERATING_SYSTEM "GNU/Linux" CACHE STRING "")
elseif (APPLE)
  set(OPERATING_SYSTEM "Darwin" CACHE STRING "")
elseif (UNIX)
  set(OPERATING_SYSTEM "UNIX" CACHE STRING "")
else()
  set(OPERATING_SYSTEM "Unknown" CACHE STRING "")
endif()

target_compile_definitions(uname PRIVATE
  PROGRAM_NAME="uname"
  OPERATING_SYSTEM="${OPERATING_SYSTEM}"
)

target_link_libraries(uname PRIVATE meta)
