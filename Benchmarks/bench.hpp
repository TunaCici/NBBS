/*
 * Benchmark function dedfinitions & configurations
 */

#include <iostream>
#include <fstream>

#include "nbbs.h"

#ifdef _WIN32
        #define FUNC_NAME __FUNCSIG__
#else
        #define FUNC_NAME __FUNCTION__
#endif

#define BENCH_MALLOC(size) nb_alloc(size)
#define BENCH_FREE(addr) nb_free(addr)
#define BENCH_ARENA_SIZE (4ULL * 1024 * 1024 * 1024) /* Bytes */
#define BENCH_ARENA_ALIGN (2ULL * 1024 * 1024) /* Bytes */

#define BENCH_BATCH_SIZE 1000ULL /* Allocs/frees in one iteration */
#define BENCH_STRESS_UPPER 0.95f /* Percent */
#define BENCH_STRESS_LOWER 0.05f /* Percent */
#define BENCH_STRESS_PERIOD 100 /* Millisecond */

/*
 * bench_alloc_init()
 *
 * Creates arena for the allocator & initializes it.
 */
static inline void bench_alloc_init()
{
        std::cout <<  "Initialize arena" << std::endl;
        uint8_t *arena = (uint8_t*) std::aligned_alloc(
                BENCH_ARENA_ALIGN, BENCH_ARENA_SIZE);
        if (!arena) {
                std::cerr << "Initialize arena fail" << std::endl;
                std::exit(1);
        }
        std::cout <<  "Initialize arena ok" << std::endl;

        /* Allocator init */
        std::cout << "Initialize allocator" << std::endl;
        if (nb_init((uint64_t) arena, BENCH_ARENA_SIZE) != 0) {
                std::cerr << "Initialize allocator fail" << std::endl;
                std::exit(1);
        }
        std::cout << "Initialize allocator ok" << std::endl;
}

/*
 * Abbrevations
 *
 * rnd: random
 * multi: multi-threaded
 * single: single-threaded
 * 
 * ofs: output file stream
 * tc: thread count
 * dur: duration in seconds
 */

int alloc_rnd_multi(std::ofstream& ofs, unsigned dur, unsigned tc);
int alloc_rnd_single(std::ofstream& ofs, unsigned dur);
int alloc_seq_multi(std::ofstream& ofs, unsigned dur, unsigned tc);
int alloc_seq_single(std::ofstream& ofs, unsigned dur);

int free_rnd_multi(std::ofstream& ofs, unsigned tc);
int free_rnd_single(std::ofstream& ofs);
int free_seq_multi(std::ofstream& ofs, unsigned tc);
int free_seq_single(std::ofstream& ofs);

int stress_multi(std::ofstream& ofs, unsigned dur, unsigned tc);
int stress_single(std::ofstream& ofs, unsigned dur);

