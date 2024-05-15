#
# Makefile
#
# Author: Tuna CICI
#

# Project
PROJECT_NAME 	= NBBS
PROJECT_DIR 	= ${shell pwd}
BUILD_DIR 	= .
TEST_DIR	= .

# Colored printing
RED 	= \033[0;31m
GREEN 	= \033[0;32m
YELLOW 	= \033[0;33m
BLUE 	= \033[0;34m
MAGENTA = \033[0;35m
CYAN 	= \033[0;36m
NC	= \033[0m

# Platform and target architecture
PLATFORM =
TARGET_ARCH =
ifeq (${OS}, Windows_NT)
	PLATFORM = ${OS}
else
	PLATFORM = $(shell uname -s)
endif

# Compiler
CC =
AR =
ifeq (${PLATFORM}, Windows_NT)
	CC = msvc
	AR = lib
else ifeq (${PLATFORM}, Darwin)
	CC = clang
	AR = ar
else ifeq (${PLATFORM}, Linux)
	CC = gcc
	AR = ar
else
	@echo ""
endif

# Flags
INCLUDES = \
	-I .
CCFLAGS = ${INCLUDES} -mno-outline-atomics -O3 \
	-Wall -Wextra -std=gnu99
ARFLAGS = rcs

# Project source files
SRCS = \
	NBBS.c
OBJS = ${SRCS:.c=.o}

default: all

%.o: %.c
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"

${PROJECT_NAME}.a: ${OBJS}
	@echo "AR ${PROJECT_NAME}.a"
	@${AR} ${ARFLAGS} ${PROJECT_NAME}.a ${OBJS}
	@echo "AR ${PROJECT_NAME}.a ${GREEN}ok${NC}"

compiledb:
	@echo "COMPILEDB -n make all"
	@compiledb -n make all
	@echo "COMPILEDB -n make all ${GREEN}ok${NC}"

all:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "CC: ${shell ${CC} --version | head -n 1}"

	@echo "--------------------- ${BLUE} BUILD LIBRARY ${NC} ---------------------"
	@mkdir -p ${BUILD_DIR}
	@${MAKE} ${PROJECT_NAME}.a

	@echo "---------------------------------------------------------"
	@echo "Build ${GREEN}complete${NC}. Enjoy life <3"

clean:
	@echo "Delete all object files (*.o)"
	@find ${BUILD_DIR} -name "*.o" -type f -delete
	@echo "Delete  all object files (*.o) ${GREEN}ok${NC}"

	@echo "Delete library (${PROJECT_NAME}.a)"
	@find ${BUILD_DIR} -name ${PROJECT_NAME}.a -type f -delete
	@echo "Delete library (${PROJECT_NAME}.a ${GREEN}ok${NC}"

	@echo "Delete 'compile_commands.json'"
	@find . -name "compile_commands.json" -type f -delete
	@echo "Delete 'compile_commands.json' ${GREEN}ok${NC}"
