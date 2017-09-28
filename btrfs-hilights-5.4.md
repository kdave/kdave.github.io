---
layout: page
category: b
tags: [ btrfs, kernel ]
feed: pko
date: 2020-02-15
status: finished
title: "Btrfs hilights in 5.4: tree checker updates"
---

A bit more detailed overview of a btrfs update that I find interesting, see the
[pull request](https://git.kernel.org/linus/7d14df2d280fb7411eba2eb96682da0683ad97f6)
for the rest.

There's not much to show in this release. Some users find that good too, a boring release. But still there are some changes of interest. The 5.4 is a long-term support stable tree, stability and core improvements are perhaps more appropriate than features that need a release or two to stabilize.

? stable not known in advance so not pushing half-baked features to stable, possibly requiring more intrusive fixups

The development cycle happened over summer and this slowed down the pace of patch reviews and update turnarounds. 

## Tree-checker updates

The tree-checker is a sanity checker of metadata that are read from/written to devices. Over time it's being enhanced by more checks, let's have a look at two of them.

### ROOT_ITEM checks

The item represents root of a b-tree, of the internal or the subvolume trees.

Let's take an example from `btrfs inspect dump-tree`:

```
       item 0 key (EXTENT_TREE ROOT_ITEM 0) itemoff 15844 itemsize 439
                generation 5 root_dirid 0 bytenr 30523392 level 0 refs 1
                lastsnap 0 byte_limit 0 bytes_used 16384 flags 0x0(none)
                uuid 00000000-0000-0000-0000-000000000000
                drop key (0 UNKNOWN.0 0) level 0
```

Some of the metadata inside the item allow only simple checks, following [commit 259ee7754b6793](https://git.kernel.org/linus/259ee7754b6793af8bdd77f9ca818bc41cfe9541):

* `key.objectid` must match the tree that's being read, though the code verifies only if the type is not 0
* `key.offset` must be 0
* block offset `bytenr` must be aligned to sector size (4KiB in this case)
* `itemsize` depends on the item type, but for the root item it's fixed value
* `level` and `drop_level` is 0 to 7, but it's not possible to cross check if the tree has really of that level
* `generation` must be lower than the super block generation, same for `lastsnap` 
* `flags` can be simply compared to the bit mask of allowed flags, right now there are two, one represents a read-only subvolume and another a subvolume that has been marked as deleted but its blocks not yet cleaned

The `refs` is a reference counter and sanity check would require reading all the expected reference holders, `bytes_used` would need to look up the block that it accounts, etc. The subvolume trees have more data like `ctime`, `otime` and real `uuid`s. If you wonder what's `byte_limit`, this used to be a mechanism to emulate quotas by setting the limit value, but it has been deprecated and unused for a long time. One day we might to find another purpose for the bytes.

Many of the tree-checker enhancements are follow ups to fuzz testing and reports, as it was in this case. The [bug report](https://bugzilla.kernel.org/show_bug.cgi?id=203261) shows that some of the incorrect data were detected and even triggered auto-repair (as this was on filesystem with DUP metadata) but there was too much damage and it crashed at some point. The crash was not random but a BUG_ON of an unexpected condition, that's sanity check of last resort. Catching inconsistent data early with a graceful error handling is of course desired and ongoing work.

### Extent metadata item checks

There are two item types that represent extents and information about sharing. `EXTENT_ITEM` is older and bigger  while `METADATA_ITEM` is the building block of `skinny-metadata` feature, smaller and more compact. Both items contain type of reference(s) and the owner (a tree id). Besides the generic checks that also the root item does (alignment, value ranges, generation), there's a number of allowed combinations of the reference types and extent types. The [commit f82d1c7ca8ae1bf](https://git.kernel.orgl/linus/f82d1c7ca8ae1bf89e8d78c5ecb56b6b228c1a75) implements that, however further explanation is out of scope of the overview as the sharing and references are the fundamental design of btrfs.

Example of `METADATA_ITEM`:

```
        item 170 key (88145920 METADATA_ITEM 0) itemoff 10640 itemsize 33
                refs 1 gen 27 flags TREE_BLOCK
                tree block skinny level 0
                tree block backref root FS_TREE
```

And `EXTENT_ITEM`:

```
        item 27 key (20967424 EXTENT_ITEM 4096) itemoff 14895 itemsize 53
                refs 1 gen 499706 flags DATA
                extent data backref root FS_TREE objectid 8626071 offset 0 count 1
```

This for a simple case with one reference, tree (for metadata) and ordinary data, so comparing the sizes shows 20 bytes saved. On my 20GiB root partition with about 70 snapshots there are XXX EXTENT and YYY METADATA items.

Otherwise there can be more references inside one item (eg. many snapshots of a file that is randomly updated over time) so the overhead of the item itself is smaller
