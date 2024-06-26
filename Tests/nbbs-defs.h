#include <cmath>
#include <cstdint>

/* Memory arena information:    */
/* ---------------------------- */
/* Total memory:         64 MiB */
/* Min/alloc/page size:   4 KiB */
/* Max order:                 9 */
/* Depth/level:              14 */
/* Base level:                5 */
/* Max size:              2 MiB */
/* ---------------------------- */

static constexpr uint64_t nbbs_total_memory = 64 * 1024 * 1024;
static constexpr uint64_t nbbs_min_size = 4 * 1024;
static constexpr uint32_t nbbs_max_order = 9;

static const uint32_t nbbs_depth = std::log2(nbbs_total_memory/nbbs_min_size);
static const uint32_t nbbs_base_level = nbbs_depth - nbbs_max_order;
static const uint32_t nbbs_max_size = std::exp2(nbbs_max_order)*nbbs_min_size;

/* Multi thread information:    */
/* ---------------------------- */
/* Thread count:              8 */
/* Iteration count:       1,000 */
/* ---------------------------- */

static constexpr auto nbbs_thread_count = 8;
static constexpr auto nbbs_iter_count = 1000;
