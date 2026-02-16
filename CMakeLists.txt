cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME "PlanetarySystem")

# Find dependencies
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

# Collect all source files in src/ folder
file(GLOB SOURCES "src/*.cpp" "src/*.c")

# Include header files from the include/ folder
include_directories(include)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    glfw
    OpenGL::GL
    ${FREETYPE_LIBRARIES}
)

# Include directories for FreeType and the include/ folder
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${FREETYPE_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include  # Include the headers in the include/ directory
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows-specific options
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux-specific options
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macOS-specific options
endif()

