include(ExternalProject)

ExternalProject_Add(
  docopt
  GIT_REPOSITORY  "https://github.com/docopt/docopt.cpp.git"
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
)

ExternalProject_Get_Property(docopt binary_dir source_dir)
add_library(docopt_s STATIC IMPORTED)
set_target_properties(docopt_s  PROPERTIES IMPORTED_LOCATION ${binary_dir}/libdocopt.a)
add_dependencies(docopt_s docopt)
include_directories(${source_dir})

ExternalProject_Add(
  json
  URL "https://github.com/nlohmann/json/archive/v1.1.0.tar.gz"
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
)
ExternalProject_Get_Property(json source_dir)
include_directories(${source_dir}/src)

ExternalProject_Add(
  easyloggingpp
  URL "https://github.com/easylogging/easyloggingpp/releases/download/v9.80/easyloggingpp_v9.80.tar.gz"
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
)
ExternalProject_Get_Property(easyloggingpp source_dir)
include_directories(${source_dir})

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBRARIES OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(runcpp runc.cc container.cc spec.cc process.cc)
add_dependencies(runcpp easyloggingpp json docopt)

set_property(TARGET runcpp PROPERTY CXX_STANDARD 14)

target_link_libraries(runcpp docopt_s stdc++fs)

install(TARGETS runcpp DESTINATION bin)
