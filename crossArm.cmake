set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(gcc_dir
	"C:/Program Files (x86)/GNU Arm Embedded Toolchain/9 2020-q2-update")
set(CMAKE_C_COMPILER ${gcc_dir}/bin/arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER ${gcc_dir}/bin/arm-none-eabi-g++.exe)

set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

set(arm_flag 
	"-mthumb -mcpu=cortex-m3")
