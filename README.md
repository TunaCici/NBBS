!["NBBS Header"](/Media/NBBS_Header.png)

Non-Blocking Buddy System allocator for multi-core machines. Based on the [research done by R. Marotta et al. titled 'NBBS' in 2021](https://ieeexplore.ieee.org/document/9358002). 

This repository includes the C implementation, Google Test units and basic benchmarks for the NBBS algorithm.
It is meant to be used as a [page-frame allocator](https://wiki.osdev.org/Page_Frame_Allocation) (a.k.a. [physical memory allocator](https://www.kernel.org/doc/gorman/html/understand/understand009.html)) or, *as the original work puts it*, a back-end allocator for system-level programs.

# Highlights

NBBS, is based on the binary buddy allocation algorithm (first described by [Kenneth C. Knowlton (published 1965)](https://dl.acm.org/doi/10.1145/365628.365655)). It focuses upon the concurrent allocations/releases done on the state of the buddy system. Instead of relying on *spin-locks*, the algorithm uses indivudual hardware *atomic instructions (Read-Modify-Write*) to provide synchronization across multiple cores.

Below is a list of highlights of the NBBS:

* **Non-Blocking Design**: Avoids spin-locks; thus, concurrent accesses from different cores won't block each other. This greatly `reduces latency` of allocation and release operations. See the [Benchmark](#benchmark) section.
* **Cache Play**: Tree & index data structure are implemented as a continuous heap in memory. This and the algorithm meta-data being physically close to each other in memory help `reduce cache misses`.
* **Memory Overhead**: The size of the data structures depends on the *total arena size* and the desired *minimum allocation size*. The following formulas can be use to calculate the data structure size. 
  * **nb_tree_size:** `2 * (arena_size / min_alloc_size) * 1 bytes`
  * **nb_index_size:** `(arena_size / min_alloc_size) * 4 bytes`
  * *For example; 6 GiB arena size /w 4 KiB minimum allocation size would cost ~9 MiB or `~0.14%` of the arena size*

For more information refer to the original research on *ieeexplore*: [NBBS: A Non-Blocking Buddy System for Multi-Core Machines
](https://ieeexplore.ieee.org/document/9358002)


# Vision

While I was designing [WesterOS](https://github.com/TunaCici/WesterOS)'s kernel, I realized a relatively big security risk.
The physical memory manager (PMM) I was using required read/write access for all the blocks it managed.
Since my kernel is responsible for all the memory, this meant that it would be identity mapped and have full access over the system memory.
And I did not really like the idea of kernel having the whole memory mapped in it's address space.

So, I decided to move on from the old PMM in an effort to find an ideal successor. An ideal PMM for WesterOS's kernel would have:

* **Static Data Structure**
  * It should have a static and complete data structure; within one memory chunk
  * Current one uses the whole memory as the data structure (e.g., [freelist](https://www.kernel.org/doc/gorman/html/understand/understand009.html#:~:text=struct%20are%20simply%3A-,free_list,-A%20linked%20list))
* **Low Memory Overhead**
  * It should not waste too much memory for large arena sizes
  * Current one does not have much memory overhead, but if I were to put the linked list inside a heap the overhead would become ~5%
* **Thread Safety**
  * It should play nice with multiple threads & cores as to be future-proof for WesterOS
  * Current one is not designed to be thread-safe

After a long research and asking around communities, I found [NBBS](https://ieeexplore.ieee.org/document/9358002).
It was a good match for what I was looking for.
However, the [reference implementation](https://github.com/HPDCS/NBBS) wasn't really plug-and-play and did stuff not mentioned by the original research.
I then decided to implement it myself and share it with others.
This way people would have access to an open-source NBBS allocator where they can just plug it into their systems without much effort.

# Algorithm

My implementation closely follows the original algorithm. So, I'll not be explaining how NBBS works & is structured. Refer to the original research on *ieeexplore*: [NBBS: A Non-Blocking Buddy System for Multi-Core Machines
](https://ieeexplore.ieee.org/document/9358002)

One concept that I have added that's not present in the original work is `blocks` (as seen in [Linux - Physical Page Allocation](https://www.kernel.org/doc/gorman/html/understand/understand009.html)). The NBBS algorithm uses the term `nodes` to refer to different areas of memory. Similiarly, each block [in Linux] represents an area of memory. So, they are practiaclly the same thing. However, I did not want to call them nodes as to keep the terminology in-line with Linux and other Unix-like operating systems.

NBBS | Linux |
:--- | :--- |
Min_size | Page size |
Node | Block |
Level* | Order* |
Base Level | Max Order |

The above terms are practically the same thing in terms of implementation except the Level*. Each node in the tree has a level ranging from 0 to `depth` (both inclusive). The size of the memory area the node is responsible of, decreases as the level increases. Comperatively each block in the tree has an order ranging from 0 to `max_order` (both inclusive). The size of the block increases as it's order increases. 

Below diagram visualizes the difference between NBBS's level and Linux's order.

!["NBBS and Linux Term Comparison"](/Media/NBBS_Linux.png)

Public APIs are not affected from this terminology interchange. I have defined the following macro for users to configure in place of the base_level as seen in the original work.

```c
#define NB_MAX_ORDER 9U
```

This value is used to set the base_level inside the initialization function as the following:

```c
uint32_t nb_base_level = nb_depth - NB_MAX_ORDER;
```

The above additions are the only deviations that I did from the original algorithm. Again, refer to the original work to learn all about NBBS.

# Benchmark

I have prepared a few benchmarks in the [Benchmarks](https://github.com/TunaCici/NBBS/tree/main/Benchmarks) folder. It's written in C++.
They are meant to measure to latency of `nb_alloc()` and `nb_free()` under different scenariosw with each having both single and multi-thread versions.

1. **Allocate sequantially:** Allocates all block on the same order (`max_order`) is chosen. Multi-threaded version divides all block equally with each other.
2. **Allocate randomly:** Allocates random blocks on different orders until memory is full. In multi-threaded version, each thread races with each other. No equal division between them occurs.
3. **Free sequantially:** Performs sequantial allocation. And then frees all of them. Multi-threaded version divides all blocks equally with each other.
4. **Free randomly:** Performs random allocation. And then frees all of them. In multi-threaded version, each thread races with each other for allocation and then waits for others to finish. After the allocation, each races to free.
5. **Stress:** Perform allocation until memory usage is at 95% and then frees them until 5% is reached. Repeats until `--duration` time runs out.

You can build, run and then plot the results on Linux or macOS systems with the following:

```bash
# 1. Build the 'bench' CLI tool
make bench

# 2. Run the desired benchmark (use --help for options)
./bench --stress --multi --threads 8 --duration 60 --output results.txt

# 3. Setup the Python virtual environment (you can also use system-wide pkgs)
cd Benchmarks
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# 4. Plot the results (the plot will automatically show up on the browser)
python3 graph.py --input ../results.txt 
```

I ran some benchmarks on my Intel i5 6600K /w 16 GB RAM machine running Ubuntu 24.04 LTS using the below tools:

* **NBBS bench CLI**
* **Cachegrind (from Valgrind)**
* **Linux Perf**

> The bench CLI is compiled using GCC 13.2.0 with `-m64` and the `-O3` optimization flag.

```bash
./bench --stress --multi --threads 8 --duration 30
```
!["Multi-threaded Stress Benchmark"](/Media/Stress_Multi_8T_20S.png)

```bash
./bench --alloc-rnd --multi --threads 8
```
!["Multi-threaded Allocation /w Random Blocks Benchmark](/Media/Alloc_RND_8T.png)

```bash
valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes ./bench --stress --multi --threads 8 --duration 30
```

```bash
==26473== 
==26473== I refs:        40,802,035
==26473== I1  misses:        34,093
==26473== LLi misses:         3,258
==26473== I1  miss rate:       0.08%
==26473== LLi miss rate:       0.01%
==26473== 
==26473== D refs:        17,006,639  ( 7,322,012 rd   + 9,684,627 wr)
==26473== D1  misses:       350,466  (   109,331 rd   +   241,135 wr)
==26473== LLd misses:       114,192  (     8,584 rd   +   105,608 wr)
==26473== D1  miss rate:        2.1% (       1.5%     +       2.5%  )
==26473== LLd miss rate:        0.7% (       0.1%     +       1.1%  )
==26473== 
==26473== LL refs:          384,559  (   143,424 rd   +   241,135 wr)
==26473== LL misses:        117,450  (    11,842 rd   +   105,608 wr)
==26473== LL miss rate:         0.2% (       0.0%     +       1.1%  )
==26473== 
==26473== Branches:      10,894,118  (10,624,759 cond +   269,359 ind)
==26473== Mispredicts:      294,676  (   225,059 cond +    69,617 ind)
==26473== Mispred rate:         2.7% (       2.1%     +      25.8%   )
```

```bash
perf stat ./bench --stress --multi --threads 8 --duration 30
```

```bash
 Performance counter stats for './bench --stress --multi --threads 8 --duration 30':

            400.57 msec task-clock                       #    0.013 CPUs utilized             
             5,655      context-switches                 #   14.117 K/sec                     
             1,141      cpu-migrations                   #    2.848 K/sec                     
             1,793      page-faults                      #    4.476 K/sec                     
       286,203,554      cycles                           #    0.714 GHz                       
       133,573,698      instructions                     #    0.47  insn per cycle            
        23,634,136      branches                         #   59.001 M/sec                     
         1,825,902      branch-misses                    #    7.73% of all branches           

      31.013083699 seconds time elapsed

       0.178297000 seconds user
       0.256657000 seconds sys
```

# Usage

To use the NBBS allocator in a C/C++ project you need to put [nbbs.h](https://github.com/TunaCici/NBBS/blob/main/nbbs.h) and [nbbs.c](https://github.com/TunaCici/NBBS/blob/main/nbbs.c) into your builds.
After that, you probably would want to configure the following in `nbbs.h`:

* `NB_MIN_SIZE`: Minimum allocation size in bytes. (e.g., 4096, 16384 or 65536)
* `NB_MAX_ORDER`: Maximum order, which defines the maximum allocation size (e.g., 10, 12, 16)
* `NB_MALLOC()`: Allocator that is needed for `nb_tree` and `nb_index` data structures

The first value `NB_MIN_SIZE` depends greatly on your project & design goal.
It's generally set to [Translation granule](https://developer.arm.com/documentation/101811/0103/Translation-granule) on Aarch64 platforms and [Page size](https://en.wikipedia.org/wiki/Page_(computer_memory)#Page_size) on others like x86 & AMD64.

Max order, again, depends on your design goals. NBBS combines this value with `NB_MIN_SIZE` to calculate the maximum allocation size.
Maximum allocation size is calculated using the formula: `(2^NB_MAX_ORDER) * NB_MIN_SIZE`. Higher the order, higher the maximum allocation size.
[On Linux this is set to 9*](https://www.kernel.org/doc/gorman/html/understand/understand009.html#:~:text=pages%2C%20where%20the-,MAX_ORDER,-is%20currently%20defined). (*actually it is 10, but it's not inclusive)

The last one `NB_MALLOC()` requires a bit more attention. The size required by NBBS data structures depends on the arena size.
Larger arena size, means larger data structures. On most systems, arena size is not known during compilation.
So, NBBS cannot pre-allocate & initialize it's data strucutres. Therefore, it relies on another allocator for it's initialization.
For example, Linux have what it's [bootmem](https://www.kernel.org/doc/html/v4.19/core-api/boot-time-mm.html) to handle these kind of requests.
You need to provide an allocator for the NBBS by defining `NB_MALLOC()`.

> If you somehow know what your system arena size will be, then you can modify the `nb_init()` to have it's data structures initialized.
> To do this, calculate the required size for `nb_tree()` and `nb_index` using the above formulas. Then, have them point to the arena you created. Don't forget to modify the `nb_tree_size` and `nb_index_size` as well.

## Unit Tests

I have provided some basic Google Test units in this repository. You can use them as an example to how to setup, use and test on your projects and system.

They are built and tested on the following compilers and architectures. Feel free to contribute and make NBBS available on others.

Platform | Compilers | Architectures
:--- | :--- | :--- 
macOS | Clang, GCC | arm64
Linux | Clang, GCC | arm64, x64

Follow the below steps to run the unit tests on Linux or macOS platforms.

```bash
# 0. Make sure 'git', 'make' and 'gcc | clang' is installed

# 1. Clone this repository
git clone https://github.com/TunaCici/NBBS.git

# 2. Init git submodules (e.g., Google Test)
git submodule update --init --recursive

# 3. Build unit tests; this will automatically run the tests
make test

# X. (Optionally) Run the ./all_test with other Google Test parameters
./all_test --gtest_repeat=1000
```

# API

The following sections describe the public and statistics APIs.
There are also lots of helper functions and private APIs that are not described here to simplify things.
You can check them out in the source code if you so desire. They are prefixed with `__` or implemented as `static inline` functions.

## Initialize

```c
int nb_init(uint64_t base, uint64_t size)
```

Initializes the NBBS to keep track of a memory region ranging from `base` to `base+size` (only `base` is inclusive). The initialization process is as follows:

1. Setup up meta-data info (e.g., `nb_depth`, `nb_base_address`)
2. Calculate the required memory size for the `nb_tree` and `nb_index` data structures
3. Try to allocate memory using `NB_MALLOC()` as defined in `nbbs.h`
4. Initialize both `nb_tree`, `nb_index` and `nb_stat_alloc_blocks` as `0x0`

Arguments:
* `uint64_t base`: Base address of the arena
* `uint64_t size`: Size of the arena

Returns a non-zero value to indicate an error if:
* `base` or `size` is `0`
* `size` is smaller than `NB_MIN_SIZE`

Otherwise, returns `0` to indicate initialization was successfull.

## Allocate

```c
void* nb_alloc(uint64_t size)
```

Allocates a memory block with the specified `size`, rounding it up to the nearest upper power-of-two size. If size is `0` it is rounded up to `NB_MIN_SIZE`.

Arguments:
* `uint64_t size`: Size of the required allocation in bytes

Returns `0` to indicate an error if:
* `size` is greater than `nb_max_size`
* No free block is found at the given size

Otherwise, returns the base address of the memory block.

## Free

```c
void nb_free(void *addr)
```

Frees the memory block pointed by `addr` as it's base address. If `addr` is 0, the function does nothing. Be careful not to give an address outside the range specified in `nb_init()` as it does not perform a range check. It is undefined behaviour if you decide to do so.

Returns nothing.

## Statistics

```c
uint64_t nb_stat_min_size();
```

Returns the minimum allocation size allowed (a.k.a. page size) defined by the user as `NB_MIN_SIZE` in `nbbs.h`. 


```c
uint32_t nb_stat_max_order();
```

Returns the maximum order, which defines the largest allocation size allowed as defined by the user as `NB_MAX_ORDER` in `nbbs.h`.

```c
uint64_t nb_stat_tree_size();
```

Returns the size of the tree data structure in bytes.

```c
uint64_t nb_stat_index_size();
```

Returns the size of the index data structure in bytes.

```c
uint32_t nb_stat_depth();
```

Returns the depth of the tree.

```c
uint32_t nb_stat_base_level();
```

Returns the base level used by the NBBS.

```c
uint64_t nb_stat_max_size();
```

Returns the maximum allocation size allowed.

```c
uint32_t nb_stat_release_count();
```

Returns the number of releases done by the NBBS. The value is rounded to 0 if `uint32_t` overflows. 

```c
uint64_t nb_stat_total_memory();
```

Returns the size of the memory region (a.k.a. arena) managed by the NBBS in bytes. 

```c
uint64_t nb_stat_used_memory();
```

Returns the current amount of memory allocated in bytes.

```c
uint64_t nb_stat_block_size(uint32_t order);
```

Returns the size of a block at the specified `order`.

Arguments:
* `uint32_t order`: Order of the block size to retrieve

```c
uint64_t nb_stat_total_blocks(uint32_t order);
```

Returns the total number of blocks at the specified order.

Arguments:
* `uint32_t order`: Order of the blocks to count

```c
uint64_t nb_stat_used_blocks(uint32_t order);
```

Returns the number of used blocks at the specified order.

Arguments:
* `uint32_t order`: Order of the blocks to count

```c
uint8_t nb_stat_occupancy_map(uint8_t *buff, uint32_t order);
```

Fills the provided `buff` with the occupancy status of blocks at the specified `order`. Each byte in the buffer represents whether a block is free (value `0`) or occupied/allocated (value `1`).

The buffer size must be atleast `nb_stat_block_size(order)`. Otherwise the function has undefined behaviour.

Arguments:
* uint8_t *buff: Buffer to fill with occupancy data
* uint32_t order: The order of the blocks to check
 
Returns a non-zero value to indicate an error if:
* Buffer is `0`
* Oder is greater than `NB_MAX_ORDER`

Otherwise, returns `0` to indicate a success.

