set(CODICE
	boot/boot.c
	boot/fwh.c
	circo/circo.c
	cmsis_rtos/cmsis_os_freertos.c
	crc/crc1021.c
	diario/diario.c
	DTC/cod/dtc.c
	eep_param/cod/param.c
	LwIP_port/cod/drv.c
	LwIP_port/cod/lan87xx.c
	LwIP_port/cod/lwiport.c
	LwIP_port/cod/net_if.c
	LwIP_port/cod/net_mdma.c
	LwIP_port/cod/net_op.c
	LwIP_port/esempio/tcp_eco.c
	LwIP_port/esempio/udp_eco.c	
	ramComune/bootinfo.c
	ramComune/ramcomune.c
	tram/tram.cpp
	vp/vp.c
)

include_directories("${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include")
include_directories("${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
include_directories("${PROJECT_SOURCE_DIR}/Dispi")
include_directories("${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/LwIP/include")


add_library(lib OBJECT ${CODICE})
