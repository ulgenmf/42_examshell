# Mission: C++ classes, via the 42 exam subjects (vect2 → bigint → polyset)

## Why
Passing the 42 rank06 C++ exam requires implementing `vect2`, `bigint`, and `polyset`
under exam conditions (no reference material beyond the subject). The user does not
want surface familiarity that lets them copy a pattern — they want to fully own the
`class` concept in C++, so that under time pressure they can derive the right
constructor/operator/member forms from the rules, not from memory of having seen it
once.

## Success looks like
- User can write vect2.hpp/.cpp from the subject in test/subs.txt unaided, and explain
  *why* each construct is written the way it is (which slot, which rule) — not just that
  it compiles.
- User can name the exact rule for: constructors (default/parameterised/copy, and the
  `Type x(args)` vs `Type x = y` distinction), member vs. non-member operator overloads
  and when each is required, `operator[]` as an lvalue, pre/post increment/decrement,
  compound assignment operator chaining, `const` correctness.
- The same understanding transfers to bigint and polyset without re-deriving it from
  scratch — those are treated as the same grammar with more surface area (bigint: no
  new class mechanics, just more state; polyset: adds inheritance/abstract classes).

## Constraints
- Learner is a 42 student, not new to programming, but wants zero unexplained jargon —
  every term (e.g. "constructor") must be defined plainly the first time it's used, not
  assumed.
- Learner writes all code themselves in the real exercise files
  (`vect2/vect2.hpp`, `vect2/vect2.cpp`, compiled/run via `vect2/main.cpp`). The teacher
  explains and reviews; it does not write or edit the exercise files itself (this mirrors
  the tutor rule already in this repo's CLAUDE.md).
- Lessons must be grouped by **tightly connected concept clusters** — not one line of
  main.cpp at a time (too slow, wastes time), and not a broad grab-bag of unrelated
  mechanics either (loses the "same symbol, different slot" precision the learner wants).
  Each lesson = one cluster of genuinely related grammar.
- Teaching style follows this repo's CLAUDE.md: teach the *grammar* — the fixed slot a
  symbol sits in and the rule attached to that slot — not "what it does" prose. When two
  things collide on the same symbol (e.g. `Type x(y)` construction vs `existing.f(y)`
  method call), name the collision and disambiguate directly.

## Out of scope (for now)
- bigint and polyset's task-specific content (unsigned big-number math, tree/array bag
  data structures, inheritance mechanics) — deferred until vect2 is fully owned.
- Any C++ topics not touched by these three subjects (templates, smart pointers, STL
  containers beyond what's needed, exceptions, etc.) unless a subject requires them.
