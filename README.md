# NBBS
Non-Blocking Buddy System allocator for multithreaded applications. Based on the research done by R. Marotta et al. titled 'NBBS' in 2021. 

# Quick Access

TODO.

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

TODO.

# Benchmark

TODO.

# Build

TODO.

# API

## Initialize

TODO.

## Allocate

TODO.

## Free

TODO.

## Diagnostics

TODO.
