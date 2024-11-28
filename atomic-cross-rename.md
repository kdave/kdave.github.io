---
layout: page
category: b
tags: [ mv, util-linux ]
date: 2024-11-27
status: final
title: Atomic cross-rename of two paths
---

Since version v3.15 linux supports a file operation that will exchange any two
directory entries atomically. This needs support of the filesystem so it’s not
universal, but the commonly used filesystems can do it.

The underlying syscall is [renameat2](https://www.man7.org/linux/man-pages/man2/rename.2.html).
Obligatory [LWN article](https://lwn.net/Articles/569134/).


```c
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags);
```

The magic is in the `flags` being set to `RENAME_EXCHANGE`, the `oldpath` and
`newpath` are simply the names (can be relative or full path), with `AT_FDCWD`
for `oldfddir` and `newfddir` unless you need to something fancy. See below for
standalone working implementations in several languages. The minimal and
stripped down example:

```c
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
        return renameat2(AT_FDCWD, argv[1], ATD_FDCWD, argv[2], RENAME_EXCHANGE);
}
```

The full example below does proper checking of arguments and existence of the files
but the syscall does that by itself too. Obviously both directory entries need
to exist at the time the rename action is performed, otherwise it would be
[TOCTOU](https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use) bug.

What is a directory entry and where it works:

- regular file
- special file (socket, named pipe, …)
- directory
- symlink (the path is not resolved, so the target remains the same)
- hardlink (it’s just another name of a file)
- subvolume (on Btrfs)

Each type can be the source and target, in any combination:

```sh
$ touch file
$ mkfifo fifo
$ mkdir dir
$ ln -s file symlink
$ ln file hardlink
$ btrfs subvol create subvol
$ ls -lis
total 16
123323316 0 drwxr-xr-x 1 user users    0 Nov 21 14:35 dir
123323315 0 prw-r--r-- 1 user users    0 Nov 21 14:35 fifo
123323314 0 -rw-r--r-- 2 user users    0 Nov 21 14:35 file
123323314 0 -rw-r--r-- 2 user users    0 Nov 21 14:35 hardlink
      256 0 drwxr-xr-x 1 user users    0 Nov 21 14:35 subvol
123323317 4 lrwxrwxrwx 1 user users    4 Nov 21 14:35 symlink -> file

$ mvx dir file
$ mvx fifo subvol
$ mvx symlink hardlink
$ ls -lis
123323314 0 -rw-r--r-- 2 user users    0 Nov 21 14:35 dir
      256 0 drwxr-xr-x 1 user users    0 Nov 21 14:35 fifo
123323316 0 drwxr-xr-x 1 user users    0 Nov 21 14:35 file
123323317 4 lrwxrwxrwx 1 user users    4 Nov 21 14:35 hardlink -> file
123323315 0 prw-r--r-- 1 user users    0 Nov 21 14:35 subvol
123323314 0 -rw-r--r-- 2 user users    0 Nov 21 14:35 symlink
```

Below are implementations in some other languages using direct interfaces to
the syscalls, I haven’t found standard libraries providing the extended syntax.
Same as there’s missing support in the `mv` utility. Welp, after 10 years.

# C

```c
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

/*
 * Cross-rename of two paths: file, directory, symlink (not the target),
 * also subvolumes (on btrfs)
 *
 * Example:
 *
 *    file - a file
 *    dir  - a directory
 *
 *    $ mvx file dir
 *
 *    file - a directory named 'file'
 *    dir  - a file named 'dir'
 *
 * Q: Should this be implemented by mv?
 * A: Yes.
 */

int main(int argc, char **argv)
{
        int ret;
        struct stat st;

        if (argc != 3) {
                fprintf(stderr, "Usage: mvx path1 path2\n");
                return 1;
        }
        if (stat(argv[1], &st) < 0) {
                fprintf(stderr, "stat: failed for %s: %m\n", argv[1]);
                return 1;
        }
        if (stat(argv[2], &st) < 0) {
                fprintf(stderr, "stat: failed for %s: %m\n", argv[2]);
                return 1;
        }

        ret = renameat2(AT_FDCWD, argv[1], AT_FDCWD, argv[2], RENAME_EXCHANGE);
        if (ret < 0)
                fprintf(stderr, "renameat2: failed with %m\n");
        return ret;
}
```

# Python

I found project [python-renameat2](https://github.com/jordemort/python-renameat2) implementing
that as a proper module
([example](https://python-renameat2.readthedocs.io/en/latest/examples.html#atomically-swap-two-files)).
Otherwise you can use this:

```python
import ctypes
import sys

libc = ctypes.CDLL("libc.so.6")
libc.renameat2.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_uint]
libc.renameat2.restype = ctypes.c_int
AT_FDCWD = -100
RENAME_EXCHANGE = (1 << 1)
path1 = str.encode(sys.argv[1])
path2 = str.encode(sys.argv[2])
ret = libc.renameat2(AT_FDCWD, path1, AT_FDCWD, path2, RENAME_EXCHANGE)
```

# Perl

```perl
use strict;
use warnings;

require("syscall.ph");
my ($path1, $path2) = @ARGV;
my $AT_FDCWD = -100;
my $RENAME_EXCHANGE = (1 << 1);
my $ret = syscall(&SYS_renameat2, $AT_FDCWD, $path1, $AT_FDCWD, $path2, $RENAME_EXCHANGE);
```
