set(SRCS true false)

foreach (util ${SRCS})
  add_executable(${util} ${util}.c)
  target_link_libraries(${util} PRIVATE meta)
  target_compile_definitions(${util} PRIVATE
    PROGRAM_NAME="${util}"
  )
endforeach ()
