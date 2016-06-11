cmake_minimum_required(VERSION 3.3.0)

project(runcpp)

if (WITH_TEST)
  include(CTest)

  add_subdirectory(src)
  add_subdirectory(test)
  enable_testing()
else(WITH_TEST)
  add_subdirectory(src)
endif(WITH_TEST)
