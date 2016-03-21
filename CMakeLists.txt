cmake_minimum_required(VERSION 3.5.0)

project(runcpp)

add_subdirectory(src)

if (BUILD_TEST)
  add_subdirectory(test)
endif(BUILD_TEST)
