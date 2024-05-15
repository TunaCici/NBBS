/*
 * This file (NBSS.c) implements the Non-Blocking Buddy System
 */

#ifndef NBBS_H
#define NBBS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "nbbs.h"

/* Configuration */
static const uint64_t nb_min_size = 4096; /* bytes */
static const uint64_t nb_max_order = 9;

/* Metadata */
static uint8_t *nb_tree = 0;
static uint32_t *nb_index = 0;

static uint64_t nb_tree_size = 0; /* bytes */
static uint64_t nb_index_size = 0; /* bytes */

static uint64_t nb_base_address = 0;
static uint64_t nb_total_memory = 0;
static uint32_t nb_depth = 0;
static uint64_t nb_base_level = 0;
static uint64_t nb_max_size = 0;
static uint32_t nb_release_count = 0;

int nb_init(uint64_t base, uint64_t size)
{
        if (base == 0 || size == 0) {
                return 1;
        }

        if (size < nb_min_size) {
                return 1;
        }

        /* Setup */
        nb_base_address = base;       
        nb_total_memory = size;
        nb_depth = LOG2_LOWER(nb_total_memory / nb_min_size);
        nb_base_level = nb_depth - nb_max_order;
        nb_max_size = EXP2(nb_max_order) * nb_min_size;

        /* Calculate required tree size - root node is at index 1  */
        uint32_t total_nodes = EXP2(nb_depth + 1);

        /* Calculate required index size */
        uint32_t total_pages = (nb_total_memory / nb_min_size);

        nb_tree_size = total_nodes * 1;  // each node is 1 byte
        nb_index_size = total_pages * 4; // each leaf index is 4 byte

        /* Allocate */
        nb_tree = (uint8_t*) malloc(nb_tree_size);
        if (!nb_tree) {
                return 1;
        }

        nb_index = (uint32_t*) malloc(nb_index_size);

        if (!nb_index) {
                return 1;
        }

        /* Initialize */
        memset((void*) nb_tree, 0x0, nb_tree_size);
        memset((void*) nb_index, 0x0, nb_index_size);

        return 0;
}

uint32_t __nb_try_alloc(uint32_t node)
{
        /* Occupy the node */
        uint8_t free = 0;
        if (!BCAS(&nb_tree[node], &free, BUSY)) {
                return node;
        }

        uint32_t current = node;
        uint32_t child = 0;

        /* Propagate the info about the occupancy up to the ancestor node(s) */
        while (nb_base_level < nb_level(current)) {
                child = current;
                current = current >> 1;

                uint8_t curr_val = 0;
                uint8_t new_val = 0;

                do {
                        curr_val = nb_tree[current];

                        if (curr_val & OCC) {
                                __nb_freenode(node, nb_level(child));
                                return current;
                        }

                        new_val = nb_clean_coal(curr_val, child);
                        new_val = nb_mark(new_val, child);
                } while (!BCAS(&nb_tree[current], &curr_val, new_val));
        }

        return 0;
}

void* nb_alloc(uint64_t size)
{
        if (nb_max_size < size) {
                return 0;
        }

        if (size < nb_min_size) {
                size = nb_min_size;
        }

        nb_alloc_again:;
        uint32_t ts = nb_release_count;
        uint32_t level = LOG2_LOWER(nb_total_memory / size);

        if (nb_depth < level) {
                level = nb_depth;
        }

        /* Range of nodes at target level */
        uint32_t start_node = EXP2(level);
        uint32_t end_node = EXP2(level + 1);

        for (uint32_t i = start_node; i < end_node; i++) {
                if (nb_is_free(nb_tree[i])) {
                        uint32_t failed_at = __nb_try_alloc(i);

                        if (!failed_at) {
                                /* TODO: Explain what's going on here */
                                uint32_t leaf = nb_leftmost(
                                        i, nb_depth) - EXP2(nb_depth); 
                                nb_index[leaf] = i;
                                
                                return (void*)
                                        (nb_base_address + leaf * nb_min_size);
                        } else {
                                /* Skip the entire subtree [of failed] */
                                uint32_t curr_level = nb_level(i);
                                uint32_t fail_level = nb_level(failed_at);

                                uint32_t d = EXP2(curr_level - fail_level);
                                i = (failed_at + 1) * d;
                        }
                }
        }

        /* A release occured, try again */
        if (ts != nb_release_count) {
                goto nb_alloc_again;
        }

        return (void*) 0;
}

void __nb_unmark(uint32_t node, uint32_t upper_bound)
{
        uint32_t current = node;
        uint32_t child = 0;

        uint8_t curr_val = 0;
        uint8_t new_val = 0;

        do {
                child = current;
                current = current >> 1;

                do {
                        curr_val = nb_tree[current];

                        if (!nb_is_coal(curr_val, child)) {
                                return;
                        }
                        
                        new_val = nb_unmark(curr_val, child);
                } while (!BCAS(&nb_tree[current], &curr_val, new_val));
        } while (upper_bound < nb_level(current) &&
                        !nb_is_occ_buddy(new_val, child));
}

void __nb_freenode(uint32_t node, uint32_t upper_bound)
{
        /* TODO: should I check for double frees? */
        if (nb_is_free(nb_tree[node])) {
                return;
        }

        /* Phase 1. Ancestors of the node are marked as coalescing */
        uint32_t current = node >> 1;
        uint32_t child = node;

        while (nb_base_level < nb_level(child)) {
                uint8_t curr_val = 0;
                uint8_t new_val = 0;
                uint8_t old_val = 0;
                
                do {
                        curr_val = nb_tree[current];
                        new_val = nb_set_coal(curr_val, child);
                        old_val = VCAS(&nb_tree[current], &curr_val, new_val);
                } while (old_val != curr_val);
                
                if (nb_is_occ_buddy(old_val, child) && 
                        nb_is_coal_buddy(old_val, child)) {
                        break;
                }

                child = current;
                current = current >> 1;
        }

        /* Phase 2. Mark the node as free */
        nb_tree[node] = 0;

        /* Phase 3. Propagate node release upward and possibly merge buddies */
        if (nb_level(node) != nb_base_level) {
                __nb_unmark(node, upper_bound);
        }
}

void nb_free(void *addr)
{
        /* Range check (is this necessary?) */
        uint64_t u_addr = (uint64_t) addr;
        if (u_addr < nb_base_address ||
                (nb_base_address + nb_total_memory) < u_addr) {
                return;
        }

        uint32_t n = (u_addr - nb_base_address) / nb_min_size;
        __nb_freenode(nb_index[n], nb_base_level);
        FAD(&nb_release_count, 1);
}

uint64_t __nb_block_size(uint32_t level)
{
        return EXP2(nb_depth - level) * nb_min_size;
}

void __nb_clean_block(void* addr, uint64_t size)
{
        if (!addr || !size) {
                return;
        }

        memset(addr, 0x0, size);
}

#endif /* NBBS_H */
