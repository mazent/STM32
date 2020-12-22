# Per compilare occorre:
#	1) creare una cartella (p.e. sorella di questa)
#	2) entrarci
#	3) eseguire: cmake-gui ..\STM32
#	4) passare alla gui il file: crossArm.cmake (cross compilazione)
#	5) compilare: make
# Per il compilation database: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..\STM32
#
# Il mio pc ha clang-tidy dentro msys2, per cui il trucco e':
#   1) commentare la parte clang-tidy
#	2) eseguire: cmake-gui ..\STM32 in cmd di window
#	3) scommentare la parte clang-tidy
#	4) compilare da dentro msys2
# Dentro msys2, per salvare su file: make 2> esito.txt
#

cmake_minimum_required(VERSION 3.7)

project(Base 
		VERSION 1.2.3.4
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
	
find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
if(CLANG_TIDY_COMMAND)
    # Le righe seguenti vengono abilitate una alla volta
	# Si passa alla successiva solo dopo aver messo a posto
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,readability-*")
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=-*,readability-*")
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,bugprone-*")
    # analisi statica
	# non ci sono le funzioni _s
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
endif()

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_C_FLAGS
	    ${arm_flag})
    set(CMAKE_CXX_FLAGS
	    ${arm_flag})
endif ()


#####################################################################
# Opzioni comuni a c e c++
set(gcc_flags "-Wall -Wextra")
if (VERS_DEBUG)
	set(gcc_flags "${gcc_flags} -g3 -O0 -Werror")
	add_definitions(-DDEBUG)
else ()
	set(gcc_flags "${gcc_flags} -O3")
	add_definitions(-DNDEBUG)
endif ()

# Altre regole utili
set(gcc_flags
	"${gcc_flags} -fstrict-aliasing -Wshadow")
set(gcc_flags
	"${gcc_flags} -Wendif-labels -Wpointer-arith -Wcast-align")

# Non si sa mai
set(gcc_flags
	"${gcc_flags} -Wno-unknown-pragmas")

#####################################################################
# Ai compilatori
set(CMAKE_C_FLAGS
	"${CMAKE_C_FLAGS} ${gcc_flags}")
set(CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} ${gcc_flags}")

#####################################################################

include_directories("${PROJECT_SOURCE_DIR}/xcmake")

add_subdirectory("cmsis.rtos")
# troppo legati all'hw
# add_subdirectory(sdram)
# add_subdirectory(uart)
# add_subdirectory(wdog)

