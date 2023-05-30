# CMakeList.txt : CMake project for process_files, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (process_files "process_files.cpp" "process_files.h" "file-cpp/file.h" "utf8-cpp/source/utf8.h" "utf8-cpp/source/utf8/checked.h" "utf8-cpp/source/utf8/core.h" "utf8-cpp/source/utf8/unchecked.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET process_files PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
