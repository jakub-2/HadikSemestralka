# CMakeList.txt : CMake project for HadikSemestralka, include source and define
# project specific logic here.
#

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Add source to this project's executable.
add_executable(HadikSemestralka "HadikSemestralka.c" "Server.h" "Server.c" "Client.h" "Client.c" "Hadik.h" "Hadik.c" "Menu.h" "Menu.c" "ClientBasic.h" "ClientBasic.c")

target_link_libraries(HadikSemestralka ${CURSES_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HadikSemestralka PROPERTY CXX_STANDARD 20)
endif()

# Add a separate executable for the server
add_executable(ServerExecutable "HadikSemestralkaServer.c" "Server.h" "Server.c" "Client.h" "Client.c" "Menu.h" "Menu.c" "Hadik.h" "Hadik.c")

target_link_libraries(ServerExecutable ${CURSES_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ServerExecutable PROPERTY CXX_STANDARD 20)
endif()

# Add a separate executable for the server
add_executable(ClientBasicExecutable "HadikSemestralkaBasic.c" "ClientBasic.h" "ClientBasic.c" "Menu.h" "Menu.c" "Hadik.h" "Hadik.c")

target_link_libraries(ClientBasicExecutable ${CURSES_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ClientBasicExecutable PROPERTY CXX_STANDARD 20)
endif()

# Add a custom target for building the server executable when running `cmake server`
add_custom_target(clientBasic ALL
  DEPENDS ClientBasicExecutable
  COMMENT "Building server executable from specified files"
)

# Add a custom target to clean up all compiled files
add_custom_target(clean-all
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/*"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/build/*"
  COMMENT "Cleaning up all build files, including compiled files"
)
# TODO: Add tests and install targets if needed.
