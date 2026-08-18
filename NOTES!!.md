# NOTES!! — personal key-point recall

Cross-project notes. Plain language, no jargon left undefined. Add to this over time.

---

## `*` means two different things depending on where it sits

This is a **slot**, not a single concept. Same symbol `*`, two unrelated rules,
picked by _where_ it appears.

**Declaration slot** — `*` right after a type name, when you are introducing a
new variable:

```cpp
int *p;       // p is a variable that holds an ADDRESS, not an int value
int x = 5;
p = &x;       // now p holds x's address
```

Rule: `Type *name;` declares `name` as a **pointer** — a variable whose stored
value is a memory address, specifically the address of some `Type`.

**Expression slot** — `*` in front of an already-declared pointer, used to
compute a value:

```cpp
std::cout << *p;   // *p means "go to the address stored in p, give me the
                    // value sitting there" — this prints 5, not an address
*p = 10;            // same idea on the left of =: "go to that address,
                    // write 10 there" — this changes x to 10
```

Rule: `*name` (no type in front) **dereferences** — follows the address to get
or set the value stored at that address.

**How to tell which slot you're in:** is there a type name immediately to the
left of `*` (`int *p`)? Declaration, making a pointer. Is there no type — just
`*` in front of a variable that already exists (`*p`)? Dereference, following
the pointer.

Your instinct ("in statements `*` acts different") is correct — sharpen it to:
declaration vs. everywhere else, not statement vs. non-statement.

Same split exists for `&`:

- Declaration slot: `int &r = x;` → `r` is a **reference** (another name for `x`, not an address).
- Expression slot: `&x` → **address-of** `x`, produces `x`'s address as a value.

---

## Why `operator[]` needs two versions (const-correctness, not read-vs-write)

Correction to your note: it is **not** "reading `value[1]` vs. assigning
`value[1] = 2`" that decides which version runs. Both of those can go through
the _same_ overload. What actually decides it is **whether the object you're
indexing into is itself declared `const`**.

```cpp
vect2 v(1, 2);          // v is NOT const
const vect2 cv(1, 2);    // cv IS const

v[0];        // uses the non-const operator[]
v[0] = 5;    // also uses the non-const operator[] — read AND write both go here
cv[0];       // uses the const operator[] — cv is const, so ONLY this overload
             // is even callable on it
cv[0] = 5;   // ERROR — the const overload returns a plain value/const
             // reference, which cannot be assigned to. Not a different
             // function being picked — this just fails to compile.
```

So the real rule: the compiler picks the overload based on the **const-ness of
the object**, not based on what you intend to do with the result. The
non-const overload happens to support both read and write because it returns
a real reference (`int&`) — a reference can be read from or written through.
The const overload only supports read because it returns a plain `int` (or
`const int&`) — nothing to write through.

```cpp
int& operator[](int index);          // callable on non-const objects; read or write
int operator[](int index) const;     // callable on const objects; read only
```

See: `lessons/0002-operator-bracket.html`, `reference/glossary.html#const-member-function`.
