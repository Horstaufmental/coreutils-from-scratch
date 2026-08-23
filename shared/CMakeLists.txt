add_library(meta STATIC
  meta.c
)

target_include_directories(meta PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
