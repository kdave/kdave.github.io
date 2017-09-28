---
layout: page
category: b
tags: [ json, c ]
date: 2023-08-09
status: finished
title: JSON format dumper (a C API)
---

We've had users asking for a machine readable output of btrfs-progs commands,
suggesting to add an alternative way to dump the data in a structured format
e.g.  json. And nowadays this is very common in many if not all commands
provided by `util-linux`. Try `lscpu --json`, or `lsblk --json`. Feeding such
output to python or a shell-friendly json parser is going to make life easier.
So that's the background motivation.

Adding that to btrfs-progs is a task for long winter evenings, but the first
step has been made. I wrote yet another json dumper. Of course this was
preceded by a research about the state of existing libraries or files to copy.
But why not have some fun writing one nevertheless.

The requirements:

* produce valid json (validators exist)
* allow nesting of compound structures
* hashes (`{key=value}`)
* arrays (`[value, value, value]`)
* simple values (integer, string, uuid, time, ...)

The nesting was one of the things I have not found in the existing libraries, or not
implemented properly. The output must be a valid json and the single most
difficult thing was the placement of continuation comma "`,`". There must be none
after the last object in the sequence. When the dump is done incrementally, the
next object is often not known while printing the current one. So the comma
cannot be printed unconditionally and previous state needs to be stored
until the last object is dumped.

But, it's not sufficient to keep track of the last object only, simply nest
some hashes and arrays. Like this:

```
 1: {
 2:    "first" : {
 3:       "nested" : {
 4:          "key" : [
 5:             "value",
 6:             "value2"
 7:          ]
 8:       }
 9:    },
10:    "last" : null
11: }
```

Last objects in sequences are on lines 6, 7, 8 and 10, so this needs a stack
for tracking that. The main building blocks for correct syntax:

- track depth of structured types
- print delayed "`,`" when there's next object in sequence
- quoting or escaping

The rest is for readable output, indentation is simply derived from the nesting
level.

The code representation of the json object should be straightforward, basically
translating the objects into API calls, for compound object there's the start
and end element. Element nesting must be proper. On the lower level, there
should be some nice specification of the key/value formatting.

I've ended up with a static declaration of the value formatting specifiers in a
list, the actual structure of the json objects will be defined by API calls
referring back to the spec structure.

```
struct rowspec {
    const char *key;
    const char *fmt;
    const char *out_text;
    const char *out_json;
};
```

The `key` refers to the key (name), the rest of the structure is relevant for
the value formatting. The `out_text` can be ignored for now, it's a
generalization of the output formatting so the same code is used for plain text
and json output, the switch is done from the outside (e.g. by a command line
option).

Basic value types are a string and number. For the purposes of btrfs-progs
there are more and so far built in the formatter, but it's extensible (on the
source level, not dynamically). Additional custom types are UUID, qgroupid (a
u64 split into 16 and 48 bits), size (human friendly formatting of byte suffix
values), and time. Another example of customization is that there are actually
two byte size formatters, one that prints "`-`" instead of zero value.

The structural API is simple:

* start/end the whole json document `fmt_start`, `fmt_end`
* start/end optionally named group,
  `fmt_print_start_group`/`fmt_print_end_group`, when the name is defined it
  means it's a map/hash `"key": "value"`, or a plain array made of "`[ value,
  ... ]`"
* for list values there's `fmt_start_value`/`fmt_end_value` so there can be not
  just a simple value but also a compound one
* and remaining is a simple value `fmt_start_value`/`fmt_end_value`

Most of them take a reference to the rowspec, this is convenient that at the
time we'd like to print the value we summon it by its name, the rest is in the
table. All the API calls take a context structure that stores the nesting level
(for indentation and sanity checks) and some internal data.

Now the above example would not be the best one to demonstrate the API but
let's try:

```
static const struct rowspec rows[] = {
    { .key = "first", .fmt = "map", .out_json = "first" },
    { .key = "nested", .fmt = "map", .out_json = "nested" },
    { .key = "key", .fmt = "list", .out_json = "key" },
    { .key = "value", .fmt = "str", .out_json = NULL },
};
struct format_ctx ctx;

fmt_start(&ctx, rows, 0, 0);
  fmt_print_start_group(&ctx, "first", JSON_TYPE_MAP);
    fmt_print_start_group(&ctx, "nested", JSON_TYPE_MAP);
      fmt_print_start_group(&ctx, "key", JSON_TYPE_ARRAY);
        fmt_print(&ctx, "value", "value");
        fmt_print(&ctx, "value", "value2");
      fmt_print_end_group(&ctx, NULL);
    fmt_print_end_group(&ctx, NULL);
  fmt_print_end_group(&ctx, NULL);
  fmt_print_start_group(&ctx, "last", JSON_TYPE_MAP);
  fmt_print_end_group(&ctx, NULL);
fmt_end(&ctx);
```

That should demonstrate it. There might be some slight tweaks needed as I took
it from a specialized use but would like to present it as a generic API. So far
it's not generic, it might become one. With sources available it's available for
further reuse, I don't plan to make it a standalone library but if there's
interest then, well, why not.

# Examples

The simple values can be formatted in multiple ways, e.g. size values with
suffixes or raw numbers, though it's defined as multiple keys:

```
    { .key = "size", .fmt = "%llu", .out_json = "size" },
    { .key = "size-si", .fmt = "size", .out_json = "size" },
    { .key = "size-si-not-zero", .fmt = "size-or-none", .out_json = "size" },
```

Raw formats starting with "`%`" are passed to `printf`, this lets it do all the
heavy work. The special formatters may need additional parameters, in this case
it's the pretty printer mode for size.

```
    if (mode == RAW)
        fmt_print(&ctx, "size", value);
    else if (mode == SI)
        fmt_print(&ctx, "size-si", value, mode);
    else if (mode == SI_NO_ZERO)
        fmt_print(&ctx, "size-si-not-zero", value, mode);
```

Here the `size-or-none` format prints "`-`" instead of `0`. As can be seen here,
not all the row specifiers need to be used at once.

# References

Source links point to the most recent release so there might be an update since
the time I write this but I'm not expecting significant changes.

* <a href="https://github.com/kdave/btrfs-progs/blob/master/common/format-output.h">https://github.com/kdave/btrfs-progs/blob/master/common/format-output.h</a>
* <a href="https://github.com/kdave/btrfs-progs/blob/master/common/format-output.c">https://github.com/kdave/btrfs-progs/blob/master/common/format-output.c</a>
* <a href="https://github.com/kdave/btrfs-progs/blob/master/tests/json-formatter-test.c">https://github.com/kdave/btrfs-progs/blob/master/tests/json-formatter-test.c</a>
* <a href="https://github.com/wireshark/wireshark/blob/master/wsutil/json_dumper.c">https://github.com/wireshark/wireshark/blob/master/wsutil/json_dumper.c</a>
