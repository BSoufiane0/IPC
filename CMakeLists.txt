cmake_minimum_required(VERSION 3.20)
project(Betriebssysteme C)

#== Funktionen einbinden ==#
include(${CMAKE_CURRENT_SOURCE_DIR}/.cmake/functions.cmake)

#== Sanitizer Auswahl ==
# Verfügbare Optionen für -DSANITIZER:
#   address   - Speicherfehler wie Use-after-free, Buffer Overflows
#   leak      - Speicherlecks
#   undefined - Undefiniertes Verhalten (z. B. Integer Overflow)
#   memory    - Uninitialisierte Speicherinhalte (nur Clang/ausgewählte Plattformen)
#   thread    - Race Conditions und Thread-Datenrennen (nicht kombinierbar mit address/leak unter GCC!)
#   none      - Deaktiviert alle Sanitizer

#== Sanitizer Flags ==#
# Standard-Sanitizer-Kombination, wenn nichts anderes gesetzt wurde
if(NOT DEFINED SANITIZER)
    set(SANITIZER "address,leak,undefined"  CACHE STRING "Sanitizer aktivieren (none,address,leak,undefined,thread,...)")
    # set(SANITIZER "address,undefined,leak,bounds,bool,vla-bound"  CACHE STRING "Sanitizer aktivieren (none,address,leak,undefined,bounds,bool,...)")
    # set(SANITIZER "thread")  # Achtung: TSAN ist unter GCC nicht mit ASAN kombinierbar!
endif()

message(STATUS "Gewählter Sanitizer: ${SANITIZER}")

# Aktivierung + Validierung + Kompatibilitätsprüfung in einer Funktion
enable_sanitizers(${SANITIZER})


#== Globale Compiler-Definitionen für alle Targets ==#
set(POSIX_VERSION "200809L")
add_compile_definitions(_POSIX_C_SOURCE=${POSIX_VERSION} _DEFAULT_SOURCE)



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
    src/util/custom/custom.c
        src/util/custom/custom.h
        src/util/wrapper/wrapper.c
        src/util/wrapper/wrapper.h

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
    NAME prak5
    SOURCES src/osmpExecutables/osmpExecutables_5.c
    NAME prak4_1
    SOURCES src/osmpExecutables/osmpExecutables_barriers_test2.c
    NAME prak4_2
    SOURCES src/osmpExecutables/osmpExecutables_barrierGatherTest.c
    NAME prak3
    SOURCES src/osmpExecutables/osmpExecutables_SendRecv.c
)

validate_executables()
create_executables()
