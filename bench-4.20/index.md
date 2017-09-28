---
layout: page
title: Some btrfs benchmarks of changes pending for 4.20
category: b
tags: [ btrfs, kernel, benchmark ]
date: 2018-10-18
update1: 2019-09-27
status: finished
---

Update: enhance the text, still a bit raw, check the numbers

Example of benchmarks of patches going to 4.20 kernel. The goal was to identify
potential differences between the devleopment branch with and without merge
to the mainline. The factors affecting the difference are usually in other
subsystems like block layer (IO queuing) or memory management (namely page
cache, memory allocator, writeback), core (scheduling, locking primitives).

* base/misc1: v4.19-rc8
* misc2: v4.19-rc8 + btrfs pull request
* misc3: v4.19 with btrfs changes merged

The numbers are reflected in the results below, referring to comparisons of
the branches. 1-to-2 is the net effect of the patches, 1-to-3 is the same
patches after merge.

The testsuite used is [MMTests](https://github.com/mgorman/mmtests), that's
internally used for performance evaluation and provides wide range of workloads

The expected performance delta should be roughly the same for 2 and 3, the
tables show if the delta is significant (green: positive, red: negative), so it
can be easily observed. Note that the values need to be properly interpreted in
the context of the benchmark and perhaps other values.

## MMTests results

- [blogbench 1, 2](blogbench/results-12)
- [blogbench 1, 2, 3](blogbench/results)
- [dbench4-async 1, 2](dbench4-async/results-12)
- [dbench4-async 1, 2, 3](dbench4-async/results)
  - good: slight improvement in 128 thread latency
  - good: decreased system cpu time
  - bad: longer overall runtime
- [dbench4-fsync 1, 2](dbench4-fsync/results-12)
- [dbench4-fsync 1, 2, 3](dbench4-fsync/results)
  - good: slight improvement in 128 thread latency
  - good: decreased system cpu time
- [filebench-webproxy 1, 2](filebench-webproxy/results-12)
- [filebench-webproxy 1, 2, 3](filebench-webproxy/results)
- [fio-async-randrw 1, 2](fio-async-randrw/results-12)
- [fio-async-randrw 1, 2, 3](fio-async-randrw/results)
- [fio-async-seqw 1, 2](fio-async-seqw/results-12)
- [fio-async-seqw 1, 2, 3](fio-async-seqw/results)
- [fsmark-metadata 1, 2](fsmark-metadata/results-12)
- [fsmark-metadata 1, 2, 3](fsmark-metadata/results)
  - good: overall improvements in transaction counts for 3

The raw stats are available.

## Test setup

- HP ProLiant DL380 G6
- 16 CPUs (Intel(R) Xeon(R), E5520 2.27GHz)
- 16 GiB RAM
- 4x SAS 136 GiB disks, partitioned
- filesystem:
  - btrfs
  - mkfs: data: raid0, metadata: raid1
  - mount: noatime
