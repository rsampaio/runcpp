cmake_minimum_required(VERSION 3.3.0)

project(runcpp)

if (NO_TEST)
  add_subdirectory(src)
else(NO_TEST)
  add_subdirectory(src)
  add_subdirectory(test)
endif(NO_TEST)
