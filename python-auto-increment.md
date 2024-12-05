---
layout: page
category: b
tags: [ python, heresy ]
date: 2024-12-11
status: draft
published: false
title: Python auto increment variables
---

I still want the unary increment/decrement operators in python, as a syntax
sugar for some specific cases. If you do not, don't read further.

Operators in python: [https://docs.python.org/3/library/operator.html](https://docs.python.org/3/library/operator.html)

Increment and decrement question
[https://stackoverflow.com/questions/1485841/behaviour-of-increment-and-decrement-operators-in-python](https://stackoverflow.com/questions/1485841/behaviour-of-increment-and-decrement-operators-in-python)
provides some insights why `++x` works but does not do what one expects.
This is parsed as `+(+(x))`. The question is now 15 years old and everybody
moved on. The unary `+` operator is an identity.

> Side quest: do you know what unary `+` does in C?

Assignment operator
[https://docs.python.org/3/reference/simple_stmts.html#grammar-token-assignment_stmt](https://docs.python.org/3/reference/simple_stmts.html#grammar-token-assignment_stmt)

Let me start with a quiz: *what could be the following class useful for?*

```python
class X():
    def __init__(self, value=0):
        self.x = value
    def __call__(self, *args):
        if len(args) == 0:
            return self.x
        self.x = args[0]
    def __int__(self):
        return int(self.x)
    def __str__(self):
        return str(self.x)
    def __pos__(self):
        self.x += 1
        return self.x - 1
    def __neg__(self):
        self.x -= 1
        return self.x + 1
```

Auto sequence is sometimes useful. Why? I've used this in GUI programming, when
dumping tabular data with custom formatting. If you write your software
correctly on the first time, congratulations. I
may not and resort to incremental development that requires lots
of editing. An example in pseudo code for Tcl/Tk application that fills rows of a
grid layout in a sequence.

```python
widget1.grid(row=0, column=0)
widget2.grid(row=1, column=0)
widget3.grid(row=2, column=0)
widget4.grid(row=3, column=0)
widget5.grid(row=4, column=0)
```

I may need to tune the layout so lines are swapped, reordered, save, run, check
and repeat until satisfied. In C++ and using e.g. Qt I'd do something like:

```cpp
row = 1;
widget3->grid(row++, 0);
widget2->grid(row++, 0);
widget5->grid(row++, 0);
widget1->grid(row++, 0);
widget4->grid(row++, 0);
```

In any way I swap the lines, they'll always be in the order as they're in the
source file and I don't have to renumber the rows each time I need to save and
run it. I could turn it to a fixed number sequence once I'm done with the
tuning. Expecting future changes I would leave it as is, extending that in the
future would be much easier.

But python religiously does not offer the `++` operators, neither pre-increment
nor post-increment. OTOH python offers other tuning methods that could be
hidden on the class level and ~~abused~~utilized as pure syntax.

Using the class from above, the python version can be rewritten like this, with
the reordered lines:

```python
row = X()
widget3.grid(+row, column=0)
widget2.grid(+row, column=0)
widget5.grid(+row, column=0)
widget1.grid(+row, column=0)
widget4.grid(+row, column=0)
```

The `+` operator `pos`, unary and returning the positive size of the value.
Overloading that to increment the value is at least a bit intuitive even for a
casual reader not familiar with the class `X`.

As a discouragement the following works (python code):

```python
++x
```

and (as implemented) increments `x` by `1`, just once. But no surprise or magic
here, standard python accepts this for any integer value. This will be first
evaluated as `+x` and then the unary `+` applied on the result, as said before
`+(+x)` so the leading `+` does not involve `x`. We can go crazy:

```python
+x
++x
+++x
++++(++++x)
+++++++++++++++x
```

All of them do single post-increment. In standard python this works for
integer-like objects too:

```python
+1
++1
+++1
++++1
```

This is evaluated as 1. Actually we can go crazy even more, because the unary
`-` and `+` can be mixed freely:

```python
+1
-1
-+1
+-1
-+-1
+-+1
-+-+1
+-+-1
```

And so on. I'll leave that as an exercise what the final value of each line
is.  This also works in C, but I digress.

What I find really bad is that `++x` does not match the pre-increment semantics
but given the limitations of the operator overloading we can't do anything
about that.

With a minor update the the class `X`, the bare value of such object instance
can be used in expressions like `x+1` or `x-1`:

```python
    def __add__(self, value):
        return self.x + value
    def __sub__(self, value):
        return self.x - value
```

and similar for the remaining integer opeators.

Heresy.
