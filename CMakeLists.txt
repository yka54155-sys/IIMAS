cmake_minimum_required(VERSION 3.10)
project(IIMAS_refactoring)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
find_package(SQLiteCpp REQUIRED)
find_package(llama REQUIRED)
find_package(OpenXLSX CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(   
    ${PROJECT_NAME} PRIVATE 
    SQLiteCpp 
    llama  
    OpenXLSX::OpenXLSX 
    OpenSSL::Crypto  
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
