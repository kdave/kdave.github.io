---
layout: page
category: b
tags: [ linux, c ]
date: 2024-11-15
status: draft
title: Linux kernel trivia - build with custom CFLAGS
---

Build linux kernel, either whole or a selected directory with compiler flags,
without editing any Makefiles or such.

    cd linux.git
    make ccflags-y='-Wundef' kernel/

Any change to flags will be detected and leads to full rebuild of all affected files.
Building whole directory must have the trailing `/`. Whole modules can be built
with `M=fs/btrfs`.

**What to use it for**

*Build test the code with different warnings* that are not on by default.
Inspiration is in manual page of `gcc`.  Linux provides sets of predefined
warning level with increasing verbosity and decreasing usefulness, see file
`scripts/Makefile.extrawarn`. The levels can be enabled on command line like

    make W=1 kernel/
    make W=12 kernel/
    make W=2 kernel/
    make W=3 kernel/

This could be quite noisy so it's better to capture that to a file and postprocess.

Capture other information during build, like `-fstack-usage` (consumption of
stack by functions, in `.su`) or `-save-temps=obj` (save generated assembly `.s` and
preprocessed sources `.i`).
