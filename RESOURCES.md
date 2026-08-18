# C++ Classes (vect2/bigint/polyset) Resources

## Knowledge

- [cppreference: Constructors and member initializer lists](https://en.cppreference.com/w/cpp/language/constructor)
  The authoritative rule set for every constructor form (default, parameterised, copy) and initialization syntax. Use for: exact rules on `Type x(args)` vs `Type x = y`, when the copy constructor is invoked implicitly.
- [cppreference: Operator overloading](https://en.cppreference.com/w/cpp/language/operators)
  Canonical reference for which operators must/can be members vs. free functions, and required signatures (e.g. `operator<<` must be a free function to put the stream on the left). Use for: every operator vect2/bigint needs.
- [cppreference: Copy assignment operator](https://en.cppreference.com/w/cpp/language/copy_assignment)
  Rule for `operator=`, implicit generation, and when to write your own. Use for: orthodox canonical form (ctor, copy ctor, copy assignment, destructor) required by polyset's subject.
- [cppreference: Increment/decrement operators](https://en.cppreference.com/w/cpp/language/operator_incdec)
  Exact rule for why post-increment takes a dummy `int` parameter and returns by value while pre-increment returns by reference. Use for: `v4++`/`++v4` in vect2's main.

## Wisdom (Communities)

- Not yet explored — user has not stated a preference on joining a community. Revisit once vect2 is passing and bigint/polyset are underway; 42 peer-review culture (evaluations) is itself a strong feedback loop.

## Gaps

- No resource yet found specifically on 42's "orthodox canonical form" convention (it's a 42-specific term, not a standard C++ one) — will define in the glossary reference doc instead of citing external material.
