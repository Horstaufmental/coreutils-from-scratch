set(SHARED_DEPS shared/encoders.c shared/decoders.c)
set(SRCS base32 base64 basenc)

foreach (base ${SRCS})
  add_executable(${base} ${base}.c ${SHARED_DEPS})
  target_compile_definitions(${base} PRIVATE
    PROGRAM_NAME="${base}"
  )
  target_include_directories(${base} PRIVATE include)
  target_link_libraries(${base} PRIVATE meta)
endforeach ()
