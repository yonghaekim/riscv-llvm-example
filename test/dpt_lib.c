#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <setjmp.h>
#include <malloc.h>

#define DPT_TAG_WIDTH 16
#define DPT_NUM_CMTS 32
#define __NR_dpt_set 436
#define __NR_arena_set 245

#ifdef __ASSEMBLY__
#define __ASM_STR(x)x
#else
#define __ASM_STR(x)#x
#endif

#ifndef __ASSEMBLY__
#define csr_write(csr, val)         \
({                \
  unsigned long __v = (unsigned long)(val);   \
  __asm__ ("csrw " __ASM_STR(csr) ", %0" \
            : : "rK" (__v)      \
            : "memory");      \
})

#define csr_read(csr)           \
({                \
  register unsigned long __v;       \
  __asm__ ("csrr %0, " __ASM_STR(csr)  \
            : "=r" (__v) :      \
            : "memory");      \
  __v;              \
})
#endif /* __ASSEMBLY__ */

// Configurable knobs via CSRs
size_t DPT_NUM_WAYS_MAX = 32;

static void *CMT; // Base addr of CMT
static size_t NUM_WAYS[DPT_NUM_CMTS]; // # of ways of CMT
static size_t *WPB = NULL; // Base addr of WPB

static size_t TEST_MODE;

static unsigned int LAST_CMT_IDX = 0;
size_t CMT_OFFSET = 0;

#define getTag(x) ((size_t) x >> (64-DPT_TAG_WIDTH))
#define maskTag(ptr) ((void *) ((size_t) ptr & 0xFFFFFFFFFFFF))

void *__tagd(void *ptr, void *tweak);
void *__xtag(void *ptr);
void __cstr(void *ptr, size_t size);
void __cclr(void *ptr);
void __bsetm(size_t *ptr);
void __bclrm(size_t *ptr);
size_t __bextm(size_t *ptr);

void __scan_bitmap(size_t *ptr, size_t size);
static void dpt_sysmalloc_hook(void *__arena_end);
static void dpt_scan_bitmap_hook(void *__ptr);

static void dpt_sysmalloc_hook(void *arena_end) {
	size_t arena_end_t = (((size_t) arena_end >> 23) & (size_t) 0xFFFF);
	//printf("Alloc new arena! CMT[%d] arena_end_t: 0x%lx\n",
	//				LAST_CMT_IDX, arena_end_t);
	switch (LAST_CMT_IDX) {
		case 0: {
			size_t csr_arena_end = (size_t) csr_read(0x440);
			//printf("[SYS-MALLOC] csr_arena_end: 0x%lx \n", csr_arena_end);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x440, csr_arena_end);
			syscall(__NR_arena_set, 0, csr_arena_end);
			//printf("new csr_arena_end: 0x%lx\n", csr_arena_end);
			break;
		} case 1: {
			size_t csr_arena_end = (size_t) csr_read(0x440);
			//printf("[SYS-MALLOC] csr_arena_end: 0x%lx \n", csr_arena_end);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x440, csr_arena_end);
			syscall(__NR_arena_set, 0, csr_arena_end);
			//printf("new csr_arena_end: 0x%lx\n", csr_arena_end);
			break;
		} case 2: {
			size_t csr_arena_end = (size_t) csr_read(0x440);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x440, csr_arena_end);
			syscall(__NR_arena_set, 0, csr_arena_end);
			break;
		} case 3: {
			size_t csr_arena_end = (size_t) csr_read(0x440);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x440, csr_arena_end);
			syscall(__NR_arena_set, 0, csr_arena_end);
			break;
		} case 4: {
			size_t csr_arena_end = (size_t) csr_read(0x441);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x441, csr_arena_end);
			syscall(__NR_arena_set, 1, csr_arena_end);
			break;
		} case 5: {
			size_t csr_arena_end = (size_t) csr_read(0x441);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x441, csr_arena_end);
			syscall(__NR_arena_set, 1, csr_arena_end);
			break;
		} case 6: {
			size_t csr_arena_end = (size_t) csr_read(0x441);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x441, csr_arena_end);
			syscall(__NR_arena_set, 1, csr_arena_end);
			break;
		} case 7: {
			size_t csr_arena_end = (size_t) csr_read(0x441);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x441, csr_arena_end);
			syscall(__NR_arena_set, 1, csr_arena_end);
			break;
		} case 8: {
			size_t csr_arena_end = (size_t) csr_read(0x442);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x442, csr_arena_end);
			syscall(__NR_arena_set, 2, csr_arena_end);
			break;
		} case 9: {
			size_t csr_arena_end = (size_t) csr_read(0x442);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x442, csr_arena_end);
			syscall(__NR_arena_set, 2, csr_arena_end);
			break;
		} case 10: {
			size_t csr_arena_end = (size_t) csr_read(0x442);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x442, csr_arena_end);
			syscall(__NR_arena_set, 2, csr_arena_end);
			break;
		} case 11: {
			size_t csr_arena_end = (size_t) csr_read(0x442);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x442, csr_arena_end);
			syscall(__NR_arena_set, 2, csr_arena_end);
			break;
		} case 12: {
			size_t csr_arena_end = (size_t) csr_read(0x443);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x443, csr_arena_end);
			syscall(__NR_arena_set, 3, csr_arena_end);
			break;
		} case 13: {
			size_t csr_arena_end = (size_t) csr_read(0x443);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x443, csr_arena_end);
			syscall(__NR_arena_set, 3, csr_arena_end);
			break;
		} case 14: {
			size_t csr_arena_end = (size_t) csr_read(0x443);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x443, csr_arena_end);
			syscall(__NR_arena_set, 3, csr_arena_end);
			break;
		} case 15: {
			size_t csr_arena_end = (size_t) csr_read(0x443);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x443, csr_arena_end);
			syscall(__NR_arena_set, 3, csr_arena_end);
			break;
		} case 16: {
			size_t csr_arena_end = (size_t) csr_read(0x444);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x444, csr_arena_end);
			syscall(__NR_arena_set, 4, csr_arena_end);
			break;
		} case 17: {
			size_t csr_arena_end = (size_t) csr_read(0x444);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x444, csr_arena_end);
			syscall(__NR_arena_set, 4, csr_arena_end);
			break;
		} case 18: {
			size_t csr_arena_end = (size_t) csr_read(0x444);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x444, csr_arena_end);
			syscall(__NR_arena_set, 4, csr_arena_end);
			break;
		} case 19: {
			size_t csr_arena_end = (size_t) csr_read(0x444);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x444, csr_arena_end);
			syscall(__NR_arena_set, 4, csr_arena_end);
			break;
		} case 20: {
			size_t csr_arena_end = (size_t) csr_read(0x445);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x445, csr_arena_end);
			syscall(__NR_arena_set, 5, csr_arena_end);
			break;
		} case 21: {
			size_t csr_arena_end = (size_t) csr_read(0x445);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x445, csr_arena_end);
			syscall(__NR_arena_set, 5, csr_arena_end);
			break;
		} case 22: {
			size_t csr_arena_end = (size_t) csr_read(0x445);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x445, csr_arena_end);
			syscall(__NR_arena_set, 5, csr_arena_end);
			break;
		} case 23: {
			size_t csr_arena_end = (size_t) csr_read(0x445);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x445, csr_arena_end);
			syscall(__NR_arena_set, 5, csr_arena_end);
			break;
		} case 24: {
			size_t csr_arena_end = (size_t) csr_read(0x446);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x446, csr_arena_end);
			syscall(__NR_arena_set, 6, csr_arena_end);
			break;
		} case 25: {
			size_t csr_arena_end = (size_t) csr_read(0x446);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x446, csr_arena_end);
			syscall(__NR_arena_set, 6, csr_arena_end);
			break;
		} case 26: {
			size_t csr_arena_end = (size_t) csr_read(0x446);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x446, csr_arena_end);
			syscall(__NR_arena_set, 6, csr_arena_end);
			break;
		} case 27: {
			size_t csr_arena_end = (size_t) csr_read(0x446);
			csr_arena_end = (csr_arena_end & ~((size_t) 0xFFFF000000000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 48));
			//csr_write(0x446, csr_arena_end);
			syscall(__NR_arena_set, 6, csr_arena_end);
			break;
		} case 28: {
			size_t csr_arena_end = (size_t) csr_read(0x447);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x000000000000FFFF));
			csr_arena_end = (csr_arena_end | (arena_end_t << 0));
			//csr_write(0x447, csr_arena_end);
			syscall(__NR_arena_set, 7, csr_arena_end);
			break;
		} case 29: {
			size_t csr_arena_end = (size_t) csr_read(0x447);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x00000000FFFF0000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 16));
			//csr_write(0x447, csr_arena_end);
			syscall(__NR_arena_set, 7, csr_arena_end);
			break;
		} case 30: {
			size_t csr_arena_end = (size_t) csr_read(0x447);
			csr_arena_end = (csr_arena_end & ~((size_t) 0x0000FFFF00000000));
			csr_arena_end = (csr_arena_end | (arena_end_t << 32));
			//csr_write(0x447, csr_arena_end);
			syscall(__NR_arena_set, 7, csr_arena_end);
			break;
		} case 31: {
			size_t csr_arena_end = (size_t) csr_read(0x447);
			syscall(__NR_arena_set, 7, csr_arena_end);
			// Last CMT is reserved to cover the rest of memory space
			break;
		} default:
			printf("Shouldn't be reached!\n");
	}

	if (LAST_CMT_IDX != (DPT_NUM_CMTS-1))
		LAST_CMT_IDX++;

	return;
}


static void dpt_scan_bitmap_hook(void *__ptr) {
  // Accessing size field in mchunk header is out-of-bounds access
  // So, need to strip the pointer first
	size_t *ptr = __xtag(__ptr);
  size_t mmaped = ((*(ptr - 1) & 0x2));
  size_t size = ((*(ptr - 1) & ~0x3) - 0x10);
#ifdef DPT_DEBUG
  printf("[scan-heap] Enter! ptr: %p size: 0x%lx\n", ptr, size);
#endif
  if (mmaped == 0)
  	__scan_bitmap(ptr, size);
}

void __scan_bitmap(size_t *ptr, size_t size) {
  size_t *start = ptr;
  size_t *end = (size_t *) ((size_t) ptr + size);
	size_t *aligned_start_64B = ((size_t *) ((size_t) ptr & (~0x3F)));
	size_t *aligned_end_64B = ((size_t *) (((size_t) ptr + size) & (~0x3F))) + 8;
#ifdef DPT_DEBUG
	printf("[scan] Enter! start: %p end: %p size: 0x%lx\n", start, end, size);
#endif

	for (size_t *chunk = aligned_start_64B; chunk < aligned_end_64B; chunk += 8) {
		//size_t idx = ((size_t) chunk >> 6);
    u_int32_t idx = ((u_int32_t) chunk >> 6);
	#ifdef DPT_DEBUG
		printf("[scan] chunk: %p idx: 0x%x ", chunk, idx);
	#endif
		size_t marks_64b = WPB[idx];
	#ifdef DPT_DEBUG
		printf("marks_64b: 0x%lx\n", marks_64b);
	#endif

    void *addr_64b = (void *) chunk;
    while (marks_64b != 0) {
	    void *addr_8b = addr_64b;
      size_t marks_8b = marks_64b & 0xFF;

      //while (marks_8b != 0 && addr_8b >= (void *) start && addr_8b < (void *) end) {
      while (marks_8b != 0 && addr_8b < (void *) end) {
        size_t mark = marks_8b & 0x1;
			#ifdef DPT_DEBUG
				printf("[scan] addr_8b: %p mark: 0x%lx\n", addr_8b, mark);
			#endif

        //if (mark != 0) {
        if (mark != 0 && addr_8b >= (void *) start && addr_8b < (void *) end) {
          void *ptr_t = __tagd(addr_8b, 0);
          //void *ptr_t = __tagd(addr_8b, addr_8b);
          //printf("[scan] Clear! ptr_t: %p addr_8b: %p\n", ptr_t, addr_8b);
          __cclr(ptr_t);
          __bclrm(ptr_t);
        }

        addr_8b++;
        marks_8b >>= 1;
      }

			addr_64b += 8;
      marks_64b >>= 8;
    }
	}
}

void __dpt_set(size_t config, size_t max_num_ways, size_t test_mode, size_t spec_mode, size_t fault_mode) {
  TEST_MODE = test_mode;
	DPT_NUM_WAYS_MAX = max_num_ways;

	if (config != 0) {
		// Init CMT
		size_t max_size = 16 * ((size_t) 1 << DPT_TAG_WIDTH) * DPT_NUM_WAYS_MAX * DPT_NUM_CMTS;
		CMT = (void *) malloc(max_size);
		CMT_OFFSET = 16 * ((size_t) 1 << DPT_TAG_WIDTH) * DPT_NUM_WAYS_MAX;

    for (int i=0; i<DPT_NUM_CMTS; i++) {
    	NUM_WAYS[i] = 1;
	    //printf("[DPT] Allocate CMT[%d]: 0x%lx # Ways: %lu\n", i, (size_t) CMT + (CMT_OFFSET * i), NUM_WAYS[i]);
		}

		// Set CSR_ARENAN
		//if (TEST_MODE < 2) {
		if (TEST_MODE == 2) {
			csr_write(0x440, 0x000000000000FFFF);
		} else {
			csr_write(0x440, 0x4000400040004000);
			csr_write(0x441, 0x4000400040004000);
			csr_write(0x442, 0x4000400040004000);
			csr_write(0x443, 0x4000400040004000);
			csr_write(0x444, 0x4000400040004000);
			csr_write(0x445, 0x4000400040004000);
			csr_write(0x446, 0x4000400040004000);
			csr_write(0x447, 0x0000400040004000);
		}

		if (config == 2 || config == 3) {
			// Init WPB
			WPB = (size_t *) malloc ((size_t) 1 << 29); // 38 (vaddrSize) - 9 (512B) + 3 (8B)
			//printf("[DPT] Allocate WPB: %p\n", WPB);
		}
	}

	size_t enableDPT = (config != 0);
  size_t dpt_config = (((size_t) enableDPT << 62) | // enableDPT
												((size_t) 0x1 << 61) | // enableStats
												((spec_mode & 0x1) << 60) | // disableSpec
												((fault_mode & 0x1) << 59) | // suppressFault
												(((size_t) TEST_MODE & 0x3) << 57) | // mode
												(((size_t) 0x1) << 55) | // threshold
												(((((size_t) DPT_NUM_WAYS_MAX / 8)) & 0x3F) << 48) | // max num ways
												((size_t) CMT << 0));
  // Invoke syscall
	printf("[DPT] dpt_config: 0x%lx\n", dpt_config);
  syscall(__NR_dpt_set, dpt_config, (size_t) WPB);

	if (config != 0) {
		__dpt_sysmalloc_hook = dpt_sysmalloc_hook;
		__dpt_scan_bitmap_hook = dpt_scan_bitmap_hook;
	}

	DPT_H = (config != 0);
	DPT_F = (config == 3);
}
