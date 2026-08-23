add_executable(ls
  main.c
  longformat.c
  bytetohr.c
)
target_compile_definitions(ls PRIVATE
  PROGRAM_NAME="ls"
)
target_include_directories(ls PRIVATE include)
target_link_libraries(ls PRIVATE meta)
