#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define TAG_WIDTH 16
#define MAX_NUM_WAYS 1024
#define NUM_WAYS 1
#define __NR_dpt_set  436

// *** capability metadata table configuration *** //
// # of ways of bounds table
size_t num_ways = 0;
// Base addr of bounds table
void *CMT = NULL;

void __dpt_set() {
	// Init num_ways, CMT
	num_ways = NUM_WAYS;
	size_t max_size = 8 * ((size_t) 1 << TAG_WIDTH) * MAX_NUM_WAYS;
	CMT = (void *) malloc(max_size);
	CMT = (void *) ((size_t) CMT & 0xFFFFFFFFFFFFFF00);

	// Set CPT Config
	size_t config = (((size_t) 0x1 << 62) | // enableDPT
											((size_t) 0x1 << 61) | // enableStats
											((size_t) num_ways << 48) |
											((size_t) CMT << 0));

	printf("Init CMT! base_addr: %p # ways: %lu config: 0x%lx\n", CMT, num_ways, config);

#ifndef QEMU
	// Invoke syscall
	syscall(__NR_dpt_set, config, CMT);
#endif
}
