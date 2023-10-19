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

project(Base)

option(VERS_DEBUG 
	"Compilo in debug"
	OFF)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
	
find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
if(CLANG_TIDY_COMMAND)
    # Le righe seguenti vengono abilitate una alla volta
	# Si passa alla successiva solo dopo aver messo a posto
	set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,readability-*,-readability-magic-numbers")
	# --fix modifica i file
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=-*,readability-*,-readability-magic-numbers")
    # // NOLINTNEXTLINE(bugprone-branch-clone)
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,bugprone-*")
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,clang-analyzer-core*,cert-*")
	#set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--checks=-*,clang-analyzer-core*,performance-*")
    # analisi statica
	# non ci sono le funzioni _s
    #set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
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
# http://www.ganssle.com/tem/tem460.html#article3
set(gcc_flags "-Wconversion -Wfloat-equal -Winit-self -Wparentheses -Wredundant-decls -Wreturn-type")
set(gcc_flags "-Wswitch-default -Wuninitialized -Wunused-function -Wunused-parameter -Wunused-value")
set(gcc_flags "-Wunused-variable -pedantic")

if (VERS_DEBUG)
	set(gcc_flags "${gcc_flags} -g3 -O0 -Werror")
	add_definitions(-DUSE_FULL_ASSERT)
else ()
	set(gcc_flags "${gcc_flags} -O3")
	add_definitions(-DNDEBUG)
endif ()

# Altre regole utili
set(gcc_flags
	"${gcc_flags} -fstrict-aliasing -Wshadow")
set(gcc_flags
    "${gcc_flags} -Wendif-labels -Wpointer-arith")
#set(gcc_flags
#    "${gcc_flags} -Wcast-align")
set(gcc_flags 
	"${gcc_flags} -Wwrite-strings")

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

#add_definitions(-D) 

add_subdirectory("cmsis.rtos")
# troppo legati all'hw
# add_subdirectory(sdram)
# add_subdirectory(uart)
# add_subdirectory(wdog)



