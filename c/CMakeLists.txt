cmake_minimum_required(VERSION 3.16)
project(crop_tracking_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files (manual only — no proto gen)
set(SRC_FILES
    tracking-solution.c
)

# Executable
add_executable(tracking-solution ${SRC_FILES})

# Include current source dir (if needed for headers)
target_include_directories(tracking-solution PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tracking-solution cjson)

# Install binary into local build/bin directory (not ../bin)
install(TARGETS tracking-solution
    RUNTIME DESTINATION bin
)
