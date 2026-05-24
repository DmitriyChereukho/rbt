cmake_minimum_required(VERSION 3.16)
project(rbt)

set(CMAKE_CXX_STANDARD 20)

add_executable(rbt RBTree.cpp)

target_link_libraries(rbt PRIVATE ${CMAKE_SOURCE_DIR}/libRBTreeTest.a)
