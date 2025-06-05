cmake_minimum_required(VERSION 3.20)
project(Betriebssysteme C)

#== Funktionen einbinden ==#
include(${CMAKE_CURRENT_SOURCE_DIR}/.cmake/functions.cmake)

#== ASAN ==#
# Ändern Sie den Wert von "OFF" zu "ON", um AddressSanitizer zu aktivieren
option(USE_ASAN "AddressSanitizer aktivieren" OFF)
compile_with_asan(${USE_ASAN})

#== Compiler Flags ==#
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -Wextra -Wconversion -g")

#== Add Library target ==#
set(LIBRARY_SRC_FILES
        src/osmpLibrary/osmpLib.c
        src/osmpLibrary/osmpLib.h
        src/osmpLibrary/OSMP.h
        src/osmpExecutables/osmpExecutables_SendRecv.c
        src/osmpExecutables/osmpExecutables_barrierGatherTest.c
        src/osmpExecutables/osmpExecutables_barriers_test2.c
        src/osmpExecutables/osmpExecutables_5.c
)
add_library(libosmp STATIC ${LIBRARY_SRC_FILES})

#== Define Libraries ALL executables link against ==#
set(LIBRARIES
        pthread
        libosmp
)

#== Executables ==#
set(EXECUTABLES
        NAME osmprun
        SOURCES src/osmpRunner/osmpRun.c
        NAME echoall
        SOURCES src/osmpExecutables/echoAll.c
        NAME sendrecv
        SOURCES src/osmpExecutables/osmpExecutables_SendRecv.c

        # Neue Executables hier einfügen
        # NAME <name>
        # SOURCES <source1> <source2> ...
)

# Define the executable target
#add_executable(executable
#        src/osmpExecutables/osmpExecutables_SendRecv.c
#        src/osmpLibrary/osmpLib.c
#        src/osmpLibrary/osmpLib.h
#        src/osmpLibrary/OSMP.h
#        src/util/custom/custom.c
#        src/util/wrapper/wrapper.c
#        src/util/custom/custom.h
#        src/util/wrapper/wrapper.h
#)
#
## Define a custom command that runs the executable
#add_custom_command(
#        OUTPUT ${CMAKE_BINARY_DIR}/executable_output.txt  # Output file to track execution
#        COMMAND executable > ${CMAKE_BINARY_DIR}/executable_output.txt  # Run executable
#        DEPENDS executable  # Ensure executable is built first
#        COMMENT "Running executable to generate required outputs"
#        VERBATIM
#)

# Create a custom target that depends on the custom command
#add_custom_target(
#        run_executable ALL
#        DEPENDS ${CMAKE_BINARY_DIR}/executable_output.txt
#)

# Define the runner target
add_executable(runner
        src/osmpRunner/osmpRun.c
        src/osmpLibrary/osmpLib.c
        src/osmpLibrary/osmpLib.h
        src/osmpLibrary/OSMP.h
        src/util/custom/custom.c
        src/util/wrapper/wrapper.c
        src/util/custom/custom.h
        src/util/wrapper/wrapper.h
)

# Make runner depend on run_executable
#add_dependencies(runner run_executable)

target_link_libraries(runner ${LIBRARIES})
validate_executables()
create_executables()