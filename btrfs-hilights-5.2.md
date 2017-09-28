---
layout: page
category: b
tags: [ btrfs, kernel ]
feed: pko
date: 2019-07-25
status: finished
title: Btrfs hilights in 5.2
---

A bit more detailed overview of a btrfs update that I find interesting, see the
[pull request](https://git.kernel.org/linus/9f2e3a53f7ec9ef55e9d01bc29a6285d291c151e)
for the rest.

## Read-time, write-time corruption detection

The tree-checker is a recent addition to the sanity checks, a functionality
that verifies a subset of metadata structures. The capabilities and strength is
limited by the context and bits of information available at some point, but
still there's enough to get an additional value.

The context here means a b-tree leaf that packs several items into a block (ie.
the `nodesize`, 16KiB by default). The individual items' members' values can be
verified against allowed values, the order of the keys of all items listed in
the leaf header can be checked etc. This is just for a rough idea what happens.

With that in the works, there are two occasions that can utilize the extended
checking:

* read-time -- the first time a block is read fresh from disk
* write-time -- when the final contents of a block is in memory and going to be written to disk

It's clear that the read-time is merely a detector of problems that already
happened, so there's not much to do besides warning and returning an error
(EUCLEAN). Turning the filesystem to read-only to prevent further problems is
another option and inevitable on some occasions.

The write side check aims to catch silent errors that could make it to the
permanent storage. The reasons why this happens are two fold, but the main idea
is to catch memory bit flips. You'd be surprised how often this happens, that
would be for a separate article entirely.

The new checks in 5.2 improve:

* device item *(item that describes the physical device of the filesystem)* --
  check that items have the right key type, device id is within allowed range
  and that usage and total sizes are within limits of the physical device
* inode item *(item for inodes, ie. files, directories or special files)*-- the
  `objectid` (that would be the inode number) is in the range, generation is
  consistent with the global one and the basic sanity of mode, flags/attributes
  and link count
* block group profiles -- check that only a single type is set in the bit mask
  that represents block group profile type of a chunk (ie.
  single/dup/raid1/...)

As mentioned before, the bits to check are inside a single buffer that
represents the tree block and the check is really local to that. As an inode
item can represent more than just files and directories, doing structural
checks like where and how the inode item is linked to is not easy in this
context. This is basically what would `btrfs check` do.

The checks need to be fast because they happen on each metadata block, so no
additional IO is allowed. This still brings some overhead but is (considered)
negligible compared to all other updates to the block. A measurement can be
done by adding tracepoint, but that's left as an exercise.
