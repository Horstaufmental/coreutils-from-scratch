cmake_minimum_required(VERSION 3.10)
project(coreutils-from-scratch LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GLOBAL_VERSION 2.0) # now relying on a global version scheme rather than individual per util
add_compile_definitions(
        VERSION="${GLOBAL_VERSION}"
        # These two are guaranteed to be constant anyways
        PROJECT_NAME="coreutils from scratch"
        AUTHORS="Horstaufmental"
)

# Enable compile_commands.json generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable output of compile commands during generation." FORCE)
# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Include the shared/ subdirectory
add_subdirectory(shared)

# Include the src/ subdirectory
add_subdirectory(src)
