---
layout: page
category: b
tags: [ linux, c ]
date: 2024-11-20
status: final
title: "Linux kernel tip: build with custom CFLAGS"
---

Build linux kernel, either whole or a selected directory with compiler flags,
without editing any Makefiles or such.

```sh
    cd linux.git
    make ccflags-y='-Wundef' kernel/
```

Any change to flags will be detected and leads to full rebuild of all affected files.
Building whole directory must have the trailing `/`. Whole modules can be built
with `M=fs/btrfs`, where `M=` does not require the trailing `/`.

**What to use it for**

*Build test the code with different warnings* that are not on by default.
Inspiration is in [gcc(1) manual page](https://www.man7.org/linux/man-pages/man1/gcc.1.html)
or in the
[GCC Option Summary](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html). 
Linux provides sets of predefined warning level with increasing verbosity and
decreasing usefulness, see file [scripts/Makefile.extrawarn](https://elixir.bootlin.com/linux/latest/source/scripts/Makefile.extrawarn).
The levels can be enabled on command line like

```sh
    make W=1 kernel/
    make W=12 kernel/
    make W=2 kernel/
    make W=3 kernel/
```

This could be quite noisy so it's better to capture that to a file and postprocess.

Capture other information during build, like `-fstack-usage` (consumption of
stack by functions, in `.su`) or `-save-temps=obj` (save generated assembly `.s` and
preprocessed sources `.i`).