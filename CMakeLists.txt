cmake_minimum_required(VERSION 3.13)
project(redis-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard

# Include the directory where the Server.h file is located
include_directories(src/includes)

# Specify both the main.cpp and Server.cpp files as sources
set(SOURCE_FILES 
    src/main.cpp 
    src/includes/Server.cpp 
    src/includes/CommandHandler.cpp 
    src/includes/RespProtocol.cpp
    src/includes/InMemoryStorage.cpp
)

add_executable(server ${SOURCE_FILES})

# Find the Threads package (this will set the necessary variables for pthreads)
find_package(Threads REQUIRED)

# Link the Threads library (pthread) to the executable
target_link_libraries(server PRIVATE Threads::Threads)