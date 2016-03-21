find_package(Threads REQUIRED)

include(ExternalProject)

ExternalProject_Add(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  # Disable install step
  INSTALL_COMMAND ""
)

add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
  IMPORTED_LOCATION "${binary_dir}/googlemock/gtest/libgtest.a"
  IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)
set_target_properties(libgmock PROPERTIES
  IMPORTED_LOCATION "${binary_dir}/googlemock/libgmock.a"
  IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

include_directories(${source_dir}/googletest/include)
include_directories(${source_dir}/googlemock/include)

add_subdirectory(testruncpp)
