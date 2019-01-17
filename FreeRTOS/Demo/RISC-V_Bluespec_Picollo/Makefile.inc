# File to be included in all makefiles

RISCV_XLEN ?= 32
RISCV_LIB  ?= elf
RISCV ?=
CCPATH =

ifeq ($(RISCV_XLEN), 32)
#CROSS_COMPILE 	= riscv32-unknown-elf-
ARCH 		= -march=rv32im
ABI 		= -mabi=ilp32
endif

ifeq ($(RISCV_XLEN), 64)
#CROSS_COMPILE 	= riscv64-unknown-elf-
ARCH 		= -march=rv64imafd
#ABI 		= -mabi=lp64d
endif


TARGET=$(CCPATH)riscv${RISCV_XLEN}-unknown-${RISCV_LIB}
#-----------------------------------------------------------
GCC		= $(TARGET)-gcc
OBJCOPY	= $(TARGET)-objcopy
OBJDUMP	= $(TARGET)-objdump
AR		= $(TARGET)-ar
RANLIB	= $(TARGET)-ranlib

PROG	= riscv-spike
CRT0	= arch/boot.S

FREERTOS_SOURCE_DIR	= ../../Source
#CUNIT	= ../../../cunit

#-----------------------------------------------------------
WARNINGS= -Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
		-Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused

