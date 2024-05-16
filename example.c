#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>



#include "nbbs.h"

int main()
{
        /* 0. Create a memory arena/pool */
        void *arena = NULL;
        uint32_t arena_size = 32 * 1024 * 1024;

        arena = (void*) malloc(arena_size);
        if (!arena) {
                printf("[!] malloc(%u) fail\n", arena_size);
                return 1;
        }

        /* 1. Initialize NBBS */
        uint8_t result = nb_init((uint64_t) arena, arena_size);
        if (result != 0) {
                printf("[!] nb_init(0x%p, %u) fail\n", arena, arena_size);
                return 1;
        }

        /* 2. Allocate */
        int *my_arr = NULL;
        int my_arr_size = 2 * 1024 * 1024;

        my_arr = (int*) nb_alloc(my_arr_size);
        if (!my_arr) {
                printf("[!] nb_alloc(%d) fail\n", my_arr_size);
                return 1;
        }

        /* 3. Use it however you want */
        srand(42);
        for (int i = 0; i < my_arr_size; i++) {
                my_arr[i] = rand();
        }

        /* 4. Get diagnostics */
        printf("Total memory: %llu KiBs\n", nb_diag_total_memory() / 1024);
        printf("\tUsed: %llu \n", nb_diag_used_memory());
        printf("\tFree: %llu \n", nb_diag_total_memory() - nb_diag_used_memory());

        printf("Max [alloc] size: %llu KiBs\n", nb_diag_max_size() / 1024);
        printf("Min [alloc] size: %llu KiBs\n", nb_diag_min_size() / 1024);

        printf("Tree size: %llu KiBs\n", nb_diag_tree_size() / 1024);
        printf("Index size: %llu KiBs\n", nb_diag_index_size() / 1024);

        printf("Depth: %u\n", nb_diag_depth());
        printf("Base level: %u\n", nb_diag_base_level());
        printf("Max order: %u\n", nb_diag_max_order());
        printf("Release count: %u\n", nb_diag_release_count());

        uint32_t order = 9;
        uint32_t block_size = nb_diag_block_size(order);

        printf("Order %u has size %u KiBs\n", order, block_size / 1024);

        uint8_t *occupancy_map = NULL;
        uint32_t occupancy_map_size = nb_diag_total_blocks(order);

        occupancy_map = (uint8_t*) malloc(occupancy_map_size);
        if (!occupancy_map) {
                printf("[!] malloc(%u) fail\n", occupancy_map_size);
                return 1;
        }

        result = nb_diag_occupancy_map(occupancy_map, order);
        if (!result) {
                printf("[!] nb_diag_occupancy_map(0x%p, %u) fail\n",
                        occupancy_map, order);
                return 1;
        }

        printf("Occupancy map of order %u:\n\t", order);
        for (uint32_t i = 0; i < occupancy_map_size; i++) {
                printf("%d: ", i);
                
                if (occupancy_map[i]) {
                        printf("full, ");
                } else {
                        printf("free, ");
                }
        }
        printf("\b\b  \n");

        /* 5. Free */
        nb_free((void*) my_arr);

        return 0;
}
