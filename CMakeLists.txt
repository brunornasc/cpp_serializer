cmake_minimum_required(VERSION 4.1)
project(cpp_serializer)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp_serializer src/main.cpp
        src/FileWalker.cpp
        src/Parser.cpp
        src/CodeGenerator.cpp
        src/TypeChecker.cpp
        src/include/CodeGenerator.h
        src/include/FileWalker.h
        src/include/Parser.h
        src/include/TypeChecker.h
        src/include/Utils.h
        src/Utils.cpp
        src/include/ClassInfo.h
        examples/ClassB.h)
