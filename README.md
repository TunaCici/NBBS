!["NBBS Header"](/Media/NBBS_Header.png)

Non-Blocking Buddy System allocator for multi-core machines. Based on the [research done by R. Marotta et al. titled 'NBBS' in 2021](https://ieeexplore.ieee.org/document/9358002). 

This repository includes the C implementation, Google Test units and basic benchmarks for the NBBS algorithm.
It is meant to be used as a [page-frame allocator](https://wiki.osdev.org/Page_Frame_Allocation) (a.k.a. [physical memory allocator](https://www.kernel.org/doc/gorman/html/understand/understand009.html)) or, *as the original work puts it*, a back-end allocator for system-level programs.

# Highlights

NBBS, is based on the binary buddy allocation algorithm (first described by [Kenneth C. Knowlton (published 1965)](https://dl.acm.org/doi/10.1145/365628.365655)). It focuses upon the concurrent allocations/releases done on the state of the buddy system. Instead of relying on *spin-locks*, the algorithm uses indivudual *atomic instructions (Read-Modify-Write*) provided by the hardware.

Below is a list of highlights of the NBBS:

* **Non-Blocking Design**: Avoids spin-locks; thus, concurrent accesses from different cores won't block each other. This greatly `reduces latency` of operations like allocation and release compared to other blocking designs.
* **Cache Play**: The tree & index data structure is designed as a continuous heap. This and the algorithm meta-data being close to each other in memory helps to `reduce cache misses`.
* **Memory Overhead**: The size of the data structures maintained depends on the *total arena size* & the desired *minimum allocation size*. A rough formula to find data structure memory overhead is: 
  *  **nb_tree_size:** `2 * (arena_size / min_alloc_size) * 1 bytes`
    * **nb_index_size:** `(arena_size / min_alloc_size) * 4 bytes`
    * *For example; 6 GiB arena size /w 4 KiB minimum allocation size would cost ~9 MiB or `~0.14%`.*

For more information refer to the original research on *ieeexplore*: [NBBS: A Non-Blocking Buddy System for Multi-Core Machines
](https://ieeexplore.ieee.org/document/9358002)


# Vision

While I was designing WesterOS's kernel, I realized a relatively big security risk.
The physcical memory management system (PMM) I was using was requiring read/write access for all the blocks it was keeping track of.
This meant that my kernel would be identity mapped and have full access over the system memory.
And I don't really like the idea of kernel being in control of everything and every process.

So, I decided to ditch the old PMM in an effort to find an ideal successor. An ideal PMM for WesterOS's kernel would have:

* Static metadata structure.
    * One that does not require access to blocks. (just an observer)
* Lightweight metadata structure.
    * Shouldn't waste too much memory with large arena sizes.

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

The above terms are practically the same thing in terms of implementation except level*. Each node in the tree has a level ranging from 0 to depth (both inclusive). The size of the memory are the node responsible of decreases as the level increases. Comperatively each block in the tree has an order ranging from 0 to max_order (both inclusive). The size of the block increases as it's order increases. 

Below diagram visualizes the difference between NBBS's level and Linux's order.

!["NBBS and Linux Term Comparison"](/Media/NBBS_Linux.png)

Public API is not affected from this terminology concept. I have defined the following macro in place of the base_level as seen in the original work.

```c
#define NB_MAX_ORDER 9U
```

This value is used to set the base_level inside the initialization function as follow:

```c
uint32_t nb_base_level = nb_depth - NB_MAX_ORDER;
```

The above additions are the only deviations that I did from the original algorithm. Again, refer to the original work to learn all about NBBS.

# Benchmark

TODO.

# Usage

Tested on the following:

Platform | Compilers | Architectures
:--- | :--- | :--- 
macOS | Clang, GCC | arm64
Linux | Clang, GCC | arm64, x64 
Windows | Clang, MSVC | x64

TODO.

# API

TODO.

## Initialize

TODO.

## Allocate

TODO.

## Free

TODO.

## Statistics

TODO.
