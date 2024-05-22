#
# Makefile
#
# Author: Tuna CICI
#

# Project
PROJECT_NAME 	= NBBS
BUILD_DIR 	= Build
TEST_DIR	= Tests

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
	TARGET_ARCH = ${PROCESSOR_ARCHITECTURE}
else
	PLATFORM = $(shell uname -s)
	TARGET_ARCH = $(shell uname -m)
endif

# Compiler
CC =
CXX =
AR =
ifeq (${PLATFORM}, Windows_NT)
	CC = msvc
	CXX = msvc
	AR = lib
else ifeq (${PLATFORM}, Darwin)
	CC = clang
	CXX = clang++
	AR = ar
else ifeq (${PLATFORM}, Linux)
	CC = gcc
	CXX = g++
	AR = ar
else
	@echo ""
endif

# Flags
INCLUDES = \
	-I . -I Tests/googletest/googletest/include
CCFLAGS = ${INCLUDES} -g \
	-Wall -Wextra -std=c11
CXXFLAGS = ${INCLUDES} -g \
	-Wall -Wextra -std=c++20
ARFLAGS = rcs

# Architecture specific flags
ifeq (${TARGET_ARCH}, $(filter ${TARGET_ARCH}, arm arm64 aarch64))
	CCFLAGS += -mno-outline-atomics
else ifeq (${TARGET_ARCH}, $(filter ${TARGET_ARCH}, x86 x86_64))
	CCFLAGS +=
else
	@echo "Unkown target architecture ${TARGET_ARCH}"
	@exit 1
endif

# Project source files
SRCS = \
	nbbs.c
OBJS = ${SRCS:.c=.o}

# GoogleTest
GTEST_DIR = Tests/googletest/googletest
GTEST_HEADERS = ${GTEST_DIR}/include/gtest/*.h \
                ${GTEST_DIR}/include/gtest/internal/*.h
GTEST_SRCS = ${GTEST_DIR}/src/*.cc ${GTEST_DIR}/src/*.h ${GTEST_HEADERS}
GTEST_LIBS = libgtest.a libgtest_main.a

GTEST_CPPFLAGS = ${INCLUDES} -g -isystem ${GTEST_DIR}/include
GTEST_CXXFLAGS = ${INCLUDES} -g -Wall -Wextra -std=c++20

# Test source files
TEST_SRCS = \
	nbbs.c \
	Tests/nbbs-helpers.cpp \
	Tests/nbbs-private.cpp \
	Tests/nbbs-init.cpp \
	Tests/nbbs-statistics.cpp \
	Tests/nbbs-alloc-single.cpp \
	Tests/nbbs-free-single.cpp
TEST_OBJS := ${filter %.o, ${TEST_SRCS:.c=.o}}
TEST_OBJS += ${filter %.o, ${TEST_SRCS:.cpp=.o}}

# To switch between normal and test builds
IS_TEST ?= False

%.o: %.c
ifeq (${IS_TEST}, True)
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${TEST_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"
else
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"
endif

%.o: %.cpp
ifeq (${IS_TEST}, True)
	@echo "CXX $<"
	@${CXX} ${CXXFLAGS} -c $< -o ${TEST_DIR}/${notdir $@}
	@echo "CXX $< ${GREEN}ok${NC}"
else
	@echo "CXX $<"
	@${CXX} ${CXXFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CXX $< ${GREEN}ok${NC}"
endif

${PROJECT_NAME}.a: ${OBJS}
	@echo "AR ${PROJECT_NAME}.a"
	@${AR} ${ARFLAGS} ${PROJECT_NAME}.a ${addprefix ${BUILD_DIR}/, $(notdir ${OBJS})}
	@echo "AR ${PROJECT_NAME}.a ${GREEN}ok${NC}"

compiledb:
	@echo "COMPILEDB -n make all"
	@compiledb -n make all
	@echo "COMPILEDB -n make all ${GREEN}ok${NC}"

all:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "CC: ${shell ${CC} --version | head -n 1}"
	@echo "CXX: ${shell ${CXX} --version | head -n 1}"

	@echo "--------------------- ${BLUE} BUILD LIBRARY ${NC} ---------------------"
	@mkdir -p ${BUILD_DIR}
	@${MAKE} ${PROJECT_NAME}.a

	@echo "--------------------- ${BLUE} BUILD TESTS ${NC} ----------------------"
	@${MAKE} all_test IS_TEST=True

	@echo "---------------------------------------------------------"
	@echo "Build ${GREEN}complete${NC}. Enjoy life <3"

# Google Test libraries
libgtest.a: ${GTEST_SRCS}
	@echo "CXX $<"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest-all.cc -o ${TEST_DIR}/gtest-all.o
	@echo "CXX $< ${GREEN}ok${NC}"

	@echo "AR ${TEST_DIR}/$@"	
	@${AR} ${ARFLAGS} ${TEST_DIR}/$@ \
		${TEST_DIR}/gtest-all.o
	@echo "AR ${TEST_DIR}/$@ ${GREEN}ok${NC}"

libgtest_main.a: libgtest.a ${GTEST_SRCS}
	@echo "CXX src/gtest_main.cc"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest_main.cc -o ${TEST_DIR}/gtest_main.o
	@echo "CXX src/gtest_main.cc ${GREEN}ok${NC}"

	@echo "AR ${TEST_DIR}/$@"	
	@${AR} ${ARFLAGS} ${TEST_DIR}/$@ \
		${TEST_DIR}/gtest-all.o ${TEST_DIR}/gtest_main.o
	@echo "AR ${TEST_DIR}/$@ ${GREEN}ok${NC}"

all_test: ${TEST_OBJS} ${GTEST_LIBS}
	@echo "CXX ${addprefix ${TEST_DIR}/, $(notdir ${TEST_OBJS})} ${TEST_DIR}/libgtest_main.a"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} \
		${addprefix ${TEST_DIR}/, $(notdir ${TEST_OBJS})} \
		${TEST_DIR}/libgtest_main.a -o ${TEST_DIR}/all_test
	@echo "CXX ${TEST_OBJS} ${addprefix ${TEST_DIR}/, $(notdir ${OBJS})} ${TEST_DIR}/libgtest_main.a ${GREEN}ok${NC}"

test:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "${shell ${CC} --version | head -n 1}"
	@echo "${shell ${CXX} --version | head -n 1}"

	@echo "------------------------ ${BLUE} BUILD ${NC} ------------------------"
	@${MAKE} all_test IS_TEST=True

	@echo "------------------------ ${GREEN} TEST ${NC} ------------------------"
	@${TEST_DIR}/all_test

clean:
	@echo "Delete all object files (*.o)"
	@find ${BUILD_DIR} -name "*.o" -type f -delete
	@echo "Delete  all object files (*.o) ${GREEN}ok${NC}"

	@echo "Delete library (${PROJECT_NAME}.a)"
	@find ${BUILD_DIR} -name ${PROJECT_NAME}.a -type f -delete
	@echo "Delete library (${PROJECT_NAME}.a ${GREEN}ok${NC}"

	@echo "Delete all test files under ${TEST_DIR}"
	@find ${TEST_DIR} -name "*.o" -type f -delete
	@find ${TEST_DIR} -name "*.a" -type f -delete
	@find ${TEST_DIR} -name "${TEST_OBJS}" -type f -delete
	@find ${TEST_DIR} -name "All_Test" -type f -delete
	@echo "Delete all test files under ${TEST_DIR} ${GREEN}ok${NC}"

	@echo "Delete 'compile_commands.json'"
	@find . -name "compile_commands.json" -type f -delete
	@echo "Delete 'compile_commands.json' ${GREEN}ok${NC}"
