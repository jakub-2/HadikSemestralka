# CMakeList.txt : CMake project for HadikSemestralka, include source and define
# project specific logic here.
#

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Add source to this project's executable.
add_executable (HadikSemestralka "HadikSemestralka.c"  "Server.h" "Server.c" "Client.h" "Client.c" "Hadik.h" "Hadik.c" "Menu.h" "Menu.c")

target_link_libraries(HadikSemestralka ${CURSES_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HadikSemestralka PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
