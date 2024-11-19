---
layout: page
category: b
tags: [ asm, perl, diff ]
date: 2019-10-12
status: finished
title: Simple asm diff filter
---

One thing you windows guys have is a decent binary diff. I know there's radare2
on linux, but I'm a noob to set it up and I'd be fine with something less
fancy. Something like 2 `objdump -dr` files, intelligent filter and `vimdiff`.

The tricky part is the intelligent filter. I say *intelligent*, but in fact it's
a 20 line perl script that basically filters out things that are not important
or are likely to change in a slightly modified C source.

The result is lossy, eg. there are no addresses and thus jumps can't be
followed, but having that would actually deserve the 'intelligent' title.
My use case is simpler, eg. doing small tweaks like reordering lines, adding
annotations (like READ\_ONCE/WRITE\_ONCE) or reducing argument counts.

Which takes eg.

```
0000000000000000 <btrfs_set_lock_blocking_read>:
       0:       e8 00 00 00 00          callq  5 <btrfs_set_lock_blocking_read+0x5>
                        1: R_X86_64_PLT32       __fentry__-0x4
       5:       55                      push   %rbp
       6:       53                      push   %rbx
       7:       48 89 fb                mov    %rdi,%rbx
       a:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1)
       f:       65 8b 05 00 00 00 00    mov    %gs:0x0(%rip),%eax        # 16 <btrfs_set_lock_blocking_read+0x16>
                        12: R_X86_64_PC32       cpu_number-0x4
      16:       89 c0                   mov    %eax,%eax
      18:       48 0f a3 05 00 00 00    bt     %rax,0x0(%rip)        # 20 <btrfs_set_lock_blocking_read+0x20>
      1f:       00
                        1c: R_X86_64_PC32       __cpu_online_mask-0x4
      20:       0f 82 b5 00 00 00       jb     db <btrfs_set_lock_blocking_read+0xdb>
      26:       0f b6 ab 94 00 00 00    movzbl 0x94(%rbx),%ebp
      2d:       40 80 fd 01             cmp    $0x1,%bpl
      31:       0f 87 00 00 00 00       ja     37 <btrfs_set_lock_blocking_read+0x37>
                        33: R_X86_64_PC32       .text.unlikely+0x11
      37:       83 e5 01                and    $0x1,%ebp
      3a:       74 1b                   je     57 <btrfs_set_lock_blocking_read+0x57>
      3c:       8b 8b 88 00 00 00       mov    0x88(%rbx),%ecx
      42:       65 48 8b 04 25 00 00    mov    %gs:0x0,%rax
      49:       00 00
                        47: R_X86_64_32S        current_task
      4b:       39 88 c0 04 00 00       cmp    %ecx,0x4c0(%rax)
      51:       0f 84 bd 00 00 00       je     114 <btrfs_set_lock_blocking_read+0x114>
      57:       8b 83 18 02 00 00       mov    0x218(%rbx),%eax
      5d:       85 c0                   test   %eax,%eax
      5f:       0f 84 b2 00 00 00       je     117 <btrfs_set_lock_blocking_read+0x117>
      65:       f0 ff 83 90 00 00 00    lock incl 0x90(%rbx)
      6c:       8b 83 14 02 00 00       mov    0x214(%rbx),%eax
      72:       85 c0                   test   %eax,%eax
      74:       0f 84 ab 00 00 00       je     125 <btrfs_set_lock_blocking_read+0x125>
      7a:       f0 ff 8b 14 02 00 00    lock decl 0x214(%rbx)
      81:       48 8d bb 98 00 00 00    lea    0x98(%rbx),%rdi
      88:       5b                      pop    %rbx
      89:       5d                      pop    %rbp
      ...
```

and produces

```
0000000000000000 <btrfs_set_lock_blocking_read>:
callq  btrfs_set_lock_blocking_read
TARGET __fentry__
push   %rbp
push   %rbx
mov    %rdi,%rbx
NOP
mov    %gs:0x0(%rip),%eax
mov    %eax,%eax
bt     %rax,0x0(%rip)
jb     btrfs_set_lock_blocking_read
movzbl 0x94(%rbx),%ebp
cmp    $0x1,%bpl
ja     btrfs_set_lock_blocking_read
and    $0x1,%ebp
je     btrfs_set_lock_blocking_read
mov    0x88(%rbx),%ecx
mov    %gs:0x0,%rax
cmp    %ecx,0x4c0(%rax)
je     btrfs_set_lock_blocking_read
mov    0x218(%rbx),%eax
test   %eax,%eax
je     btrfs_set_lock_blocking_read
lock incl 0x90(%rbx)
mov    0x214(%rbx),%eax
test   %eax,%eax
je     btrfs_set_lock_blocking_read
lock decl 0x214(%rbx)
lea    0x98(%rbx),%rdi
pop    %rbx
pop    %rbp
...
```

This looks quite simple and when lined together, the diff is readable.

So here's the magic script (pardon my perl skills):

```perl
#!/usr/bin/perl

@c=<>;
foreach(@c) {
        chomp;
        s/.*R_X86_64_PLT32\s+([^+-]+)[+-].*/TARGET $1/;
        next if(/R_X86_/);
        next if(/^\s*[0-9a-f]+:\s*([0-9a-f][0-9a-f]\s)+$/);
        s/^\s*[0-9a-f]+:\s*([0-9a-f][0-9a-f]\s)+\s*//;
        s/[0-9a-f]+ <([^+]+)\+.*>$/$1/;
        s/\s+#.*$//;
        s/nopl.*/NOP/;
        s/xchg.*ax.*ax/NOP/;
        s/data16 nop/NOP/;
        s/nop/NOP/;
        print("$_\n");
}
```

Use like:

```sh
$ objdump -dr before/locking.o > before
$ objdump -dr after/locking.o > after
$ vimdiff before after
```
