---
layout: page
category: b
tags: [ c, make, fun ]
date: 2024-12-04
status: final
title: Self Makefile and C source
---

I've seen this trick long time ago, one file that's a C source and also usable as
it's own `Makefile`, also known as
[polyglot code](https://en.wikipedia.org/wiki/Polyglot_%28computing%29).
While the polyglot has probably little use in practice, the combination with
build system can be handy, to distribute a single file. Build instructions can
be of course in a comment in the file.

Let's see a minimal example:

Source:
```c
#include <stdio.h>

int main() {
        printf("Hello world!\n");
        return 0;
}
```

Makefile:
```make
CFLAGS=-Wall
hello: hello.c
	gcc -o $@ $< $(CFLAGS)
```

Combined (syntax highlighting for C):
```c
#if 0
CFLAGS=-Wall
hello: hello.c
	gcc -o $@ $< $(CFLAGS)
ifdef false
#else
#include <stdio.h>

int main() {
        printf("Hello world!\n");
        return 0;
}
#endif
#if 0
endif
#endif
```

Combined (syntax highlighting for Makefile):
```make
#if 0
CFLAGS=-Wall
hello: hello.c
	gcc -o $@ $< $(CFLAGS)
ifdef false
#else
#include <stdio.h>

int main() {
        printf("Hello world!\n");
        return 0;
}
#endif
#if 0
endif
#endif
```

There's still a problem that the embedded C code in the `ifdef false`/`endif` is
parsed by `make`, a standalone `ifdef` in the middle of the C code will
terminate the supposedly skipped part. I've tried to confuse make more but it
seems to be quite tolerant to various syntax hacks, like defining a C function
`$` and emulating some built-in function calls.

How it all works is straightforward and depends on source level compatibility
of comments and code. The C preprocessor directives are comments for `make` so
they hide the `make` directives from compiler.
