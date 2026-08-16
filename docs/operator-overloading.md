# Operator Overloading

## What actually happens

`v1 + v2` on a built-in type (`int`, `double`) is a CPU instruction the compiler
emits directly — there's nothing to look up. On a class type, `+` has no built-in
meaning. The compiler instead treats `v1 + v2` as **syntax for a function call**:
it looks for a function named `operator+` that accepts the types on each side, and
rewrites the expression as a call to it. `v1 + v2` becomes either
`v1.operator+(v2)` (if `operator+` is a member of `v1`'s class) or
`operator+(v1, v2)` (if `operator+` is a free function taking two arguments).

This means overloading an operator is *not* teaching the compiler new syntax — the
syntax (`+`, `-`, `[]`, `<<`, `++`, ...) is fixed by the language and cannot be
invented or changed. What you're doing is writing an ordinary function and giving
it one of these reserved names, so that the compiler routes the operator syntax to
your function instead of refusing to compile.

## Member function form

```cpp
class Money {
    int cents;
public:
    Money(int c) : cents(c) {}
    Money operator+(const Money& other) const {
        return Money(cents + other.cents);
    }
};
```

`a + b` becomes `a.operator+(b)`. Because it's a member function, it has an
implicit first operand: `this`, pointing at `a`. The parameter list only holds the
*right-hand* operand (`b`). This is why a member `operator+` takes exactly one
argument for a binary operator, not two — the left one is `this`.

Consequences that follow mechanically from this:

- `this` is `const Money* const` inside a `const` member function (see the
  trailing `const` above), which is why the function is marked `const`: it reads
  `this->cents` but never writes to `*this`, so it must promise not to, otherwise
  it couldn't be called on a `const Money` object at all — a `const` object may
  only have `const` member functions called on it, full stop.
- The return value is a *new* `Money`, by value, not a reference. `cents + other.cents`
  is a temporary result that doesn't correspond to any existing object's storage —
  there's no valid object to return a reference *to*. Returning by value means the
  compiler constructs the result in the caller's context (directly, via guaranteed
  copy elision in modern C++) rather than you handing back a dangling reference to
  a local variable that's about to be destroyed when the function returns.

## Free function form

Some operators can't be members, or shouldn't be:

```cpp
Money operator+(int lhs_cents, const Money& m) {
    return Money(lhs_cents + m.cents);
}
```

If `operator+` were a member of `Money`, only `Money + int` would compile —
`a + b` becomes `a.operator+(b)`, so the left operand's type is fixed as the
class the member belongs to. `5 + a` has `int` on the left, and `int` is not a
class, so it has no member functions at all — there's no `5.operator+(a)` to
call. To support the operator with a built-in type on the left, `operator+` must
be a free function taking both operands explicitly, so the compiler can match it
regardless of which side holds which type.

The canonical case for this in the exercises here: `operator<<` for printing.
`std::cout << v` is parsed as `operator<<(std::cout, v)` — the left operand is
`std::ostream`, a type you don't own and can't add member functions to. So
`operator<<` for a custom type is necessarily a free function:

```cpp
std::ostream& operator<<(std::ostream& os, const Money& m) {
    os << m.cents;
    return os;
}
```

It returns `std::ostream&` — a reference to the same stream object it was given —
specifically so that `std::cout << v << std::endl` chains: `<<` is left-associative,
so `std::cout << v` must evaluate to something you can call `<< std::endl` on
again. Returning by value here would be wrong for a different reason than before:
`std::ostream` can't be copied at all (copying "the terminal" or "this file" isn't
a coherent operation), so a reference is the only physically possible return type.

## `operator[]`

`v[0]` becomes `v.operator[](0)` — always a member (an index only means something
relative to a specific object, so the "left operand is the object" constraint
from above is never a limitation here). It's typically overloaded twice:

```cpp
int& operator[](int i);             // called when v is non-const
int  operator[](int i) const;       // called when v is const
```

The non-`const` version returns `int&` — a reference to the actual stored `int` —
specifically so `v[1] = 12;` can work: assignment needs an lvalue (something with
an address you can write to), and only a reference to the real member gives you
that. Return by value here would hand back a disposable copy; writing to a copy
changes nothing in `v`. The `const` version returns a plain `int` (a copy) because
a `const` object must not be modified through any reference obtained from it, so
handing out a writable reference would be a hole in the `const` guarantee.

This is why both overloads exist side by side: the compiler picks whichever one
matches the constness of the object at the call site, automatically.

## Pre- and post-increment

Both `++v` and `v++` call something named `operator++`, so the language needs a
way to distinguish them at the function-signature level, since they do genuinely
different things (pre returns the value *after* incrementing; post returns the
value *before*, and increments as a side effect). The distinction is an otherwise
meaningless `int` parameter, by convention:

```cpp
Money& operator++();      // prefix: ++v
Money  operator++(int);   // postfix: v++  (the `int` here is never used/named)
```

- Prefix returns `Money&` — a reference to the now-modified object itself —
  because `++v` should evaluate to the object *after* mutation, and that object
  already exists (it's `*this`), so there's nothing to copy.
- Postfix must return the value *before* mutation, but `*this` has already been
  mutated by the time you return. So postfix necessarily makes a copy of the old
  state before modifying `*this`, then returns that copy by value:

```cpp
Money Money::operator++(int) {
    Money old = *this;   // snapshot before mutation
    ++(*this);           // reuse the prefix version
    return old;           // return the snapshot, by value
}
```

This is also why post-increment is inherently more expensive than pre-increment
for class types (one extra copy) — a fact with zero relevance for `int`, which is
why `i++` in a loop is a habit that costs nothing for `int` but isn't free for
class types like iterators.

## Compound assignment (`+=`, `-=`, `*=`)

`v += w` becomes `v.operator+=(w)`. Unlike `operator+`, this one *mutates* `v` and
returns a reference to it (`Money&`), because `+=` should evaluate to the updated
object itself (so `a += b += c` works, right-to-left), and the object being
updated already exists as `*this` — again nothing to copy.

`operator+` is often implemented *in terms of* `operator+=`, because `a + b`
without mutating either operand is naturally expressed as "copy `a`, then apply
`+=` to the copy":

```cpp
Money Money::operator+(const Money& other) const {
    Money result(*this);   // copy — `this` (the original) must stay unmodified
    result += other;
    return result;
}
```

This is the general shape: compound-assignment operators mutate and return `T&`;
their non-assignment counterparts copy first, delegate to the compound version,
and return `T` by value.

## Unary minus

`-v` becomes `v.operator-()` — zero parameters, because there's only one operand
(`this`), and no `int`-trick parameter, because there's no prefix/postfix
ambiguity for a unary operator that only has one form. Same value-return logic as
`operator+`: it produces a new object, so it returns by value, not by reference.

## `==` and `!=`

```cpp
bool operator==(const Money& other) const { return cents == other.cents; }
bool operator!=(const Money& other) const { return !(*this == other); }
```

Both are `const` (comparing never modifies either side) and return `bool` by
value (a genuinely new, temporary truth value — nothing to reference). Defining
`!=` in terms of `==` (rather than repeating the member comparison) means the
actual equality logic exists in exactly one place — if it needs to change later,
there's only one function to fix.

## The pattern underneath all of it

Every choice above — member vs. free function, return by value vs. by reference,
`const` vs. non-`const` — follows from the same two questions, asked per operator:

1. **Does this operator produce a genuinely new value, or does it refer to /
   mutate something that already exists?** New value → return by value (there's
   nothing existing to point at). Existing/mutated object → return by reference
   (the object survives past the function call, so referring to it is valid and
   copying it would be wasteful).
2. **Is the left operand guaranteed to be an object of this class?** Yes → member
   function works, `this` is that operand. No (built-in type on the left, or the
   left type isn't yours to modify, like `std::ostream`) → must be a free
   function taking both operands explicitly.

Every operator in `vect2` — `+`, `-`, `*`, `[]`, `<<`, `++`, `--`, `+=`, `-=`,
`*=`, unary `-`, `==`, `!=` — is answered by applying these two questions.
