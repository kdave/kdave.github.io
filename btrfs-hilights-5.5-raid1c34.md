---
layout: page
category: b
tags: [ btrfs, kernel ]
feed: pko
date: 2020-02-02
status: finished
title: "Btrfs hilights in 5.5: 3-copy and 4-copy block groups"
---

A bit more detailed overview of a btrfs update that I find interesting, see the
[pull
request](https://git.kernel.org/linus/97d0bf96a0d0986f466c3ff59f2ace801e33dc69)
for the rest.

## New block group profiles RAID1C3 and RAID1C4

There are two new block group profiles enhancing capabilities of the RAID1
types with more copies than 2. Brief overview of the profiles is in the table
below, for table with all profiles see manual page of `mkfs.brtfs`, also
available [on
wiki](https://btrfs.wiki.kernel.org/index.php/Manpage/mkfs.btrfs#PROFILES).

| Profile | Copies | Utilization | Min devices
| :---    |   ---: |        ---: | ---:
| RAID1   |      2 |         50% | 2
| RAID1C3 |      3 |         33% | 3
| RAID1C4 |      4 |         25% | 4

The way all the RAID1 types work is that there are 2 / 3 / 4 exact copies over
all available devices. The terminology is different from linux MD RAID, that
can do any number of copies. We decided not to do that in btrfs to keep the
implementation simple. Another point for simplicity is from the users'
perspective. That RAID1C3 provides 3 copies is clear from the type. Even after
adding a new device and not doing balance, the guarantees about redundancy
still hold. Newly written data will use the new device together with 2 devices
from the original set.

Compare that with a hypothetical RAID1CN, on a filesystem with M devices (N <=
M). When the filesystem starts with 2 devices, equivalent to RAID1, adding a
new one will have mixed redundancy guarantees after writing more data. Old data
with RAID1, new with RAID1C3 -- but all accounted under RAID1CN profile. A full
re-balance would be required to make it a reliable 3-copy RAID1. Add another
device, going to RAID1C4, same problem with more data to shuffle around.

The allocation policy would depend on number of devices, making it hard for the
user to know the redundancy level. This is already the case for
RAID0/RAID5/RAID6. For the striped profile RAID0 it's not much of a problem,
there's no redundancy. For the parity profiles it's been a known problem and
new balance filter `stripe` has been added to support fine grained selection of
block groups.

Speaking about RAID6, there's the elephant in the room, trying to cover write
hole. Lack of a resiliency against 2 device damage has been bothering all of us
because of the known write hole problem in the RAID6 implementation. How this
is going to be addressed is for another post, but for now, the newly added
RAID1C3 profile is a reasonable substitute for RAID6.

### How to use it

On a freshly created filesystem it's simple:

```
# mkfs.btrfs -d raid1c3 -m raid1c4 /dev/sd[abcd]
```

The command combines both new profiles for sake of demonstration, you should
always consider the expected use and required guarantees and choose the
appropriate profiles.

Changing the profile later on an existing filesystem works as usual, you can
use:

```
# btrfs balance start -mconvert=raid1c3 /mnt/path
```

Provided there are enough devices and enough space to do the conversion, this
will go through all metadadata block groups and after it finishes, all of them
will be of the of the desired type.

### Backward compatibility

The new block groups are not understood by old kernels and can't be mounted,
not even in the read-only mode. To prevent that a new incompatibility bit is
introduced, called `raid1c34`. Its presence on a device can be checked by
`btrfs inspect-internal dump-super` in the `incompat_flags`. On a running
system the incompat features are exported in sysfs,
`/sys/fs/btrfs/UUID/features/raid1c34`.

### Outlook

There is no demand for RAID1C5 at the moment (I asked more than once). The
space utilization is low already, the RAID1C4 survives 3 dead devices so IMHO
this is enough for most users. Extending resilience to more devices should
perhaps take a different route.

With more copies there's potential for parallelization of reads from multiple
devices. Up to now this is not optimal, there's a decision logic that's
semi-random based on process ID of the btrfs worker threads or process
submitting the IO. Better load balancing policy is a work in progress and could
appear in 5.7 at the earliest (because 5.6 development is now in fixes-only
mode).

### Look back

The history of the patchset is a bit bumpy. There was enough motivation and
requests for the functionality, so I started the analysis what needs to be
done. Several cleanups were necessary to unify code and to make it easily
extendable for more copies while using the same mirroring code. In the end
change a few constants and be done.

Following with testing, I tried simple mkfs and conversions, that worked well.
Then scrub, overwrite some blocks and let the auto-repair do the work. No
hiccups. The remaining and important part was the device replace, as the
expected use case was to substitute RAID6, replacing a missing or damaged disk.
I wrote the test script, replace 1 missing, replace 2 missing. And it did not
work. While the filesystem was mounted, everything seemed OK. Unmount, check
again and the devices were still missing. Not cool, right.

Due to lack of time before the upcoming merge window (a code freeze before next
development cycle), I had to declare it not ready and put it aside. This was in
late 2018. For a highly requested feature this was not an easy decision. Should
it be something less important, the development cycle between rc1 and final
release provides enough time to fix things up. But due to the maintainer role
with its demands I was not confident that I could find enough time to debug and
fix the remaining problem. Also nobody offered help to continue the work, but
that's how it goes.

At the late 2019 I had some spare time and looked at the pending work again.
Enhanced the test script with more debugging messages and more checks. The code
worked well, the test script was subtly broken. Oh well, what a blunder. That
cost a year, but on the other hand releasing a highly requested feature that
lacks an important part was not an appealing option.

The patchset was added to 5.5 development queue at about the last time before
freeze, final 5.5 release happened a week ago.
