#
# Makefile
#
# Author: Tuna CICI
#

# Project
PROJECT_NAME 	= NBBS
BUILD_DIR 	= Build

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
	-I . \
	-I Tests/googletest/googletest/include
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

# Source files
BENCH_SRCS = \
	nbbs.c \
	Benchmarks/bench.cpp \
	Benchmarks/alloc-rnd-multi.cpp \
	Benchmarks/alloc-rnd-single.cpp \
	Benchmarks/alloc-seq-multi.cpp \
	Benchmarks/alloc-seq-single.cpp \
	Benchmarks/free-rnd-multi.cpp \
	Benchmarks/free-rnd-single.cpp \
	Benchmarks/free-seq-multi.cpp \
	Benchmarks/free-seq-single.cpp \
	Benchmarks/stress-multi.cpp \
	Benchmarks/stress-single.cpp
BENCH_OBJS := ${filter %.o, ${BENCH_SRCS:.c=.o}}
BENCH_OBJS += ${filter %.o, ${BENCH_SRCS:.cpp=.o}}

TEST_SRCS = \
	nbbs.c \
	Tests/nbbs-helpers.cpp \
	Tests/nbbs-private.cpp \
	Tests/nbbs-init.cpp \
	Tests/nbbs-statistics.cpp \
	Tests/nbbs-alloc-single.cpp \
	Tests/nbbs-free-single.cpp \
	Tests/nbbs-alloc-multi.cpp \
	Tests/nbbs-free-multi.cpp
TEST_OBJS := ${filter %.o, ${TEST_SRCS:.c=.o}}
TEST_OBJS += ${filter %.o, ${TEST_SRCS:.cpp=.o}}

# GoogleTest
GTEST_DIR = Tests/googletest/googletest
GTEST_HEADERS = ${GTEST_DIR}/include/gtest/*.h \
                ${GTEST_DIR}/include/gtest/internal/*.h
GTEST_SRCS = ${GTEST_DIR}/src/*.cc ${GTEST_DIR}/src/*.h ${GTEST_HEADERS}
GTEST_LIBS = libgtest.a libgtest_main.a

GTEST_CPPFLAGS = ${INCLUDES} -g -isystem ${GTEST_DIR}/include
GTEST_CXXFLAGS = ${INCLUDES} -g -Wall -Wextra -std=c++20

%.o: %.c
	@echo "CC $<"
	@${CC} ${CCFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CC $< ${GREEN}ok${NC}"

%.o: %.cpp
	@echo "CXX $<"
	@${CXX} ${CXXFLAGS} -c $< -o ${BUILD_DIR}/${notdir $@}
	@echo "CXX $< ${GREEN}ok${NC}"

bench: ${BENCH_OBJS}
	@echo "CXX ${addprefix ${BUILD_DIR}/, $(notdir ${BENCH_OBJS})}} -o $@"
	@${CXX} ${CXXFLAGS} \
		${addprefix ${BUILD_DIR}/, $(notdir ${BENCH_OBJS})} -o $@
	@echo "CXX ${addprefix ${BUILD_DIR}/, $(notdir ${BENCH_OBJS})} -o $@ ${GREEN}ok${NC}"

compiledb:
	@echo "COMPILEDB -n make all"
	@compiledb -n make all
	@echo "COMPILEDB -n make all ${GREEN}ok${NC}"

all:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "CC: ${shell ${CC} --version | head -n 1}"
	@echo "CXX: ${shell ${CXX} --version | head -n 1}"

	@echo "--------------------- ${BLUE} BUILD BENCHMARKS ${NC} ---------------------"
	@${MAKE} bench

	@echo "--------------------- ${BLUE} BUILD TESTS ${NC} ----------------------"
	@${MAKE} all_test

	@echo "---------------------------------------------------------"
	@echo "Build ${GREEN}complete${NC}. Enjoy life <3"

# Google Test libraries
libgtest.a: ${GTEST_SRCS}
	@echo "CXX $<"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest-all.cc -o ${BUILD_DIR}/gtest-all.o
	@echo "CXX $< ${GREEN}ok${NC}"

	@echo "AR ${BUILD_DIR}/$@"	
	@${AR} ${ARFLAGS} ${BUILD_DIR}/$@ \
		${BUILD_DIR}/gtest-all.o
	@echo "AR ${BUILD_DIR}/$@ ${GREEN}ok${NC}"

libgtest_main.a: libgtest.a ${GTEST_SRCS}
	@echo "CXX src/gtest_main.cc"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} -I${GTEST_DIR} -c \
            ${GTEST_DIR}/src/gtest_main.cc -o ${BUILD_DIR}/gtest_main.o
	@echo "CXX src/gtest_main.cc ${GREEN}ok${NC}"

	@echo "AR ${BUILD_DIR}/$@"	
	@${AR} ${ARFLAGS} ${BUILD_DIR}/$@ \
		${BUILD_DIR}/gtest-all.o ${BUILD_DIR}/gtest_main.o
	@echo "AR ${BUILD_DIR}/$@ ${GREEN}ok${NC}"

all_test: ${TEST_OBJS} ${GTEST_LIBS}
	@echo "CXX ${addprefix ${BUILD_DIR}/, $(notdir ${TEST_OBJS})} ${BUILD_DIR}/libgtest_main.a"
	@${CXX} ${GTEST_CPPFLAGS} ${GTEST_CXXFLAGS} \
		${addprefix ${BUILD_DIR}/, $(notdir ${TEST_OBJS})} \
		${BUILD_DIR}/libgtest_main.a -o all_test
	@echo "CXX ${TEST_OBJS} ${addprefix ${BUILD_DIR}/, $(notdir ${OBJS})} ${BUILD_DIR}/libgtest_main.a ${GREEN}ok${NC}"

test:
	@echo "------------------------ ${MAGENTA} BINARIES ${NC} ------------------------"
	@echo "${shell ${CC} --version | head -n 1}"
	@echo "${shell ${CXX} --version | head -n 1}"

	@echo "------------------------ ${BLUE} BUILD ${NC} ------------------------"
	@${MAKE} all_test IS_TEST=True

	@echo "------------------------ ${GREEN} TEST ${NC} ------------------------"
	@./all_test

clean:
	@echo "Delete object files (*.o)"
	@find ${BUILD_DIR} -name "*.o" -type f -delete
	@echo "Delete object files (*.o) ${GREEN}ok${NC}"

	@echo "Delete library files (*.a|*.so)"
	@find ${BUILD_DIR} -name "*.a" -type f -delete
	@find ${BUILD_DIR} -name "*.so" -type f -delete
	@echo "Delete library files (*.a|*.so) ${GREEN}ok${NC}"

	@echo "Delete bench & all_test"
	@rm -f bench
	@rm -f all_test
	@echo "Delete bench & all_test ${GREEN}ok${NC}"

