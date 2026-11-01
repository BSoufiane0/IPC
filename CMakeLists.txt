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
    NAME osmp_run
    SOURCES src/osmpRunner/osmpRun.c
        NAME osmpExecutable_ThreadTest
        SOURCES src/osmpExecutables/osmpExecutable_ThreadTest.c
        NAME osmpExecutable_DataExchange
        SOURCES src/osmpExecutables/osmpExecutable_DataExchange.c
        NAME osmpExecutable_GatherTestDummy
        SOURCES src/osmpExecutables/osmpExecutable_GatherTestDummy.c
        NAME osmpExecutable_WrongTypeTest
        SOURCES src/osmpExecutables/osmpExecutable_WrongTypeTest.c
        NAME osmpExecutable_MessageTooBigTest
        SOURCES src/osmpExecutables/osmpExecutable_MessageTooBigTest.c
        NAME osmpExecutable_ISendIRecv
        SOURCES src/osmpExecutables/osmpExecutable_ISendIRecv.c
        NAME osmpExecutable_BlockFct1
        SOURCES src/osmpExecutables/osmpExecutable_BlockFct1.c
        NAME osmpExecutable_SendRecvTest2
        SOURCES src/osmpExecutables/osmpExecutable_SendRecvTest2.c
        NAME osmpExecutable_GetFunctions
        SOURCES src/osmpExecutables/osmpExecutable_GetFunctions.c
        NAME osmpExecutable_GatherTest
        SOURCES src/osmpExecutables/osmpExecutable_GatherTest.c
        NAME osmpExecutable_BarrierTest1
        SOURCES src/osmpExecutables/osmpExecutable_BarrierTest1.c
        NAME osmpExecutable_ISendRecv
        SOURCES src/osmpExecutables/osmpExecutable_ISendRecv.c
        NAME osmpExecutable_SendIRecv
        SOURCES src/osmpExecutables/osmpExecutable_SendIRecv.c
        NAME osmpExecutable_SendRecvTest1
        SOURCES src/osmpExecutables/osmpExecutable_SendRecvTest1.c
        NAME osmpExecutable_InitTest1
        SOURCES src/osmpExecutables/osmpExecutable_InitTest1.c
        NAME osmpExecutable_SendRecv
        SOURCES src/osmpExecutables/osmpExecutable_SendRecv.c
        NAME osmpExecutable_PostBoxLimitTest
        SOURCES src/osmpExecutables/osmpExecutable_PostBoxLimitTest.c
        NAME osmpExecutable_BarrierTest2
        SOURCES src/osmpExecutables/osmpExecutable_BarrierTest2.c
        NAME osmpExecutable_BarrierGatherTest
        SOURCES src/osmpExecutables/osmpExecutable_BarrierGatherTest.c
        NAME osmpExecutable_echoall
        SOURCES src/osmpExecutables/echoall.c


)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

validate_executables()
create_executables()
