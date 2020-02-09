---
layout: page
category: b
tags: [ btrfs, snapper ]
date: 2016-11-05
update1: 2017-09-05
status: finished
title: Escaped snapper snapshots
---

Happened to me, that space on my root partition was going low, no matter what I
removed. I have `snapper` hourly snapshots enabled, tuned the daily and weekly
settings, so the usual problem of running out of space is prevented. Also
before doing a big update of Tumbleweed (openSUSE rolling distro) I check the
available space to roughly match what `zypper` says it'd need.

```
$ snapper ls
Type   | #   | Pre # | Date                             | User | Cleanup  | Description  | Userdata
-------+-----+-------+----------------------------------+------+----------+--------------+--------------
single | 0   |       |                                  | root |          | current      |
single | 395 |       | Mon 26 Sep 2016 10:30:01 PM CEST | root | timeline | timeline     |
```

So what happened? I was removing some old kernels, manually by rpm. And it
started to report write errors, no space. Indeed, df reported just 64K
available. This is within the margin of accuracy that we're able to get from
the statfs syscall.  Side note, getting this right is a lot of guesswork, so 64K
is as good as it gets.

```
$ df -h /
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda1        30G   30G   64K  86% /
```

RPM continued to spew lots of errors, I removed rest of the snapper snapshots,
no improvement. So then I resorted to the bare 'btrfs' command and found a stale
snapper snapshot. The snapshot directory itself was there, but the metadata
were lacking (a xml file). After deleting it manually, the free space jumped up
by 6GB!

```
# btrfs subvol list /
...
... .snapshots/3721/snapshot
...
```

The theory is that I have once removed all snapshots so the numbering started
from 1 again, as the current series is around ~390, while the number of stale
snapshot is much higher.

Takeaways:

* synthetic tools may not give our an accurate view: snapper requires some
  metadata describing its snapshot, which is obviously OK, given that the user
  has the freedom to create snapshots anywhere
* learn how to use the low-level tools to cross-verify the state of the system

Summary of used tools:

* `snapper ls` -- show all snapper snapshots (for the first config)
* `snapper delete` -- delete snapshots by id or range
* `btrfs subvolume list` -- list all subvolumes (snapshots included) on a given path
* `btrfs subvolume delete` -- delete a subvolume/snapshot by path

**Update {{ page.update1 }}**

Again. There were 2 stale snapper snapshots, age of like 8 months until
discovered. The space got up by 12GiB after deletion. The root partition is
50GiB, I think it survived pretty long given the frequent updates of the rolling
distro. There's an [open issue](https://github.com/openSUSE/snapper/issues/98).
