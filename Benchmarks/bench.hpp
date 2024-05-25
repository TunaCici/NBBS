/* Benchmarks */
#include <cstdint>
#include <fstream>

#include "nbbs.h"

#define ARENA_SIZE (6ULL * 1024 * 1024 * 1024) /* Bytes */

#define BENCH_BATCH_SIZE 100ULL /* Allocs/frees in one iteration */
#define BENCH_ALLOC_SIZE (4 * 1024) /* Bytes */

#define BENCH_STRESS_UPPER 0.95f /* Percent */
#define BENCH_STRESS_LOWER 0.05f /* Percent */
#define BENCH_STRESS_PERIOD 100 /* Millisecond */
#define BENCH_STRESS_RUN 3 /* Seconds */

#define BENCH_MALLOC(size) nb_alloc(size)
#define BENCH_FREE(addr) nb_free(addr)

int alloc_rnd_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count);
int alloc_rnd_single(std::ofstream& of, unsigned long iterations);
int alloc_seq_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count);
int alloc_seq_single(std::ofstream& of, unsigned long iterations);

int free_rnd_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count);
int free_rnd_single(std::ofstream& of, unsigned long iterations);
int free_seq_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count);
int free_seq_single(std::ofstream& of, unsigned long iterations);

int stress_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count);
int stress_single(std::ofstream& of, unsigned long iterations);
