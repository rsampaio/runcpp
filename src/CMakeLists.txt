include(ExternalProject)

ExternalProject_Add(
  project_docopt
  GIT_REPOSITORY  "https://github.com/docopt/docopt.cpp.git"
  INSTALL_COMMAND ""
)

add_library(libdocopt_s STATIC IMPORTED)
ExternalProject_Get_Property(project_docopt binary_dir)
set_target_properties(libdocopt_s  PROPERTIES IMPORTED_LOCATION ${binary_dir}/libdocopt_s.a)
add_dependencies(libdocopt_s project_docopt)

# docopt.h
ExternalProject_Get_Property(project_docopt source_dir)
include_directories(${source_dir})

# libdocopt_s
add_executable(runcpp runc.cc container/container.cc)
target_link_libraries(runcpp libdocopt_s)

# C++11
set_property(TARGET runcpp PROPERTY CXX_STANDARD 11)
install(TARGETS runcpp DESTINATION bin)
