# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(rot8cpp)

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Never set in cmake file use env vars
# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_CXX_COMPILER g++)


# .ccls compile_commands.json
if (NOT EXISTS .ccls)
  file(CONFIGURE OUTPUT .ccls CONTENT compile_commands.json)
endif()
if (EXISTS .ccls)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
  message(NOTICE "Using gcc")
  add_library(compile_flags INTERFACE)
  target_compile_options(compile_flags INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Werror=vla -lstdc++ -g)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  message(NOTICE "Using g++")
  add_library(compile_flags INTERFACE)
  target_compile_options(compile_flags INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Werror=vla -g)
else()
  message(FATAL_ERROR "unknown compiler detected")
endif()

add_custom_target(run
        COMMAND ./${EXECUTABLE_NAME}
        DEPENDS ${EXECUTABLE_NAME}
        COMMENT "Running Program")

set(MY_LIBS -lm)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} compile_flags ${MY_LIBS})


set(INSTALLPATH ${CMAKE_INSTALL_BINDIR} "/usr/bin")
install(TARGETS ${EXECUTABLE_NAME}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION ${INSTALLPATH})

add_custom_target(uninstall
        COMMAND rm -rf ${INSTALLPATH}/${EXECUTABLE_NAME}
        DEPENDS ${EXECUTABLE_NAME}
        COMMENT "Installing Program")

