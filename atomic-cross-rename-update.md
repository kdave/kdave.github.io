---
layout: page
category: b
tags: [ mv, coreutils, util-linux ]
date: 2025-01-11
status: final
title: "Update: Atomic cross-rename of two paths (mv --exchange, exch)"
---

I did not notice that the cross rename already existed [when I was writing about
it](/atomic-cross-rename/) but it's been there since [coreutils v9.5](https://lists.gnu.org/archive/html/info-gnu/2024-03/msg00006.html)
released in Mar 2024.

```sh
$ mv --exchange file1 file2
```

And it also landed in [util-linux v2.40](https://github.com/util-linux/util-linux/blob/master/Documentation/releases/v2.40-ReleaseNotes)
at about the same time as utility [`exch`](https://www.man7.org/linux/man-pages/man1/exch.1.html)
(added in [commit cd094a05880c](https://github.com/util-linux/util-linux/commit/cd094a05880cf1132762c5f9724c0945557c7638)):

```sh
$ exch file1 file2
```

The two projects have an overlap in what they provide so the journey of `mv
--exchange` took a few turns:

From [coreutils git](https://github.com/coreutils/coreutils):

* 2024-02-29 [6cd2d5e5335b *`mv: add --swap (-x) option to atomically swap 2 paths`*](https://github.com/coreutils/coreutils/commit/6cd2d5e5335b5c286ff39e154e9dd38ba6923775)
* 2024-03-05 [9d8890d87231 *`mv: revert add --swap (-x) option`*](https://github.com/coreutils/coreutils/commit/9d8890d87231d477ca78269a4fce9242ec0624f1)
* 2024-03-20 [5d3fd24f4286 *`mv: new option --exchange`*](https://github.com/coreutils/coreutils/commit/5d3fd24f4286d422482dd1697c9fe1bfa30f608e)

Good that the feature is provided in standard tools but unfortunately none of
them comes with a decent description of the capabilities and limitations.  All
types of files, directories (including subvolumes) could be exchanged but only
on the same filesystem, otherwise it returns the error `EXDEV` *Invalid cross-device link*.
