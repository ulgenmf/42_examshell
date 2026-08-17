The grammar of C++ isn't a metaphor layered on top — it's the literal mechanism: a fixed set of positions, and a symbol's meaning is determined entirely by which position it sits in, never by the symbol itself. Same symbol, different slot, different meaning. That's the whole system. Here's the grammar table built directly from what's already in the two documents.

**Rule: declaration vs invocation, disambiguated by what precedes `(`**

```cpp
Type name(args);     // (1) preceded by bare type name at declaration position
existing.method(args); // (2) preceded by `.` on a name that already exists
```

Position 1 → construction: compiler looks for a matching constructor.
Position 2 → invocation: compiler looks for a matching method on `existing`'s type.
Same `(`, opposite operations. The token never tells you which — the slot does.

**Rule: `=`, disambiguated by whether the left name is new or existing**

```cpp
Type name = value;   // name doesn't exist yet → copy-init, calls copy constructor
existing = value;    // existing already has a memory slot → calls operator=
```

This is why copy constructor and copy assignment are two separate functions in OCF, not one. The grammar already splits them before you write a single line — `=` in a declaration and `=` on an existing name are different rules that happen to share a glyph.

**Rule: `&`, disambiguated by type-position vs expression-position**

```cpp
const Point& p     // sits inside a type → "reference" (alternate name for existing memory)
&x                 // sits in an expression → "address-of" (produces a pointer value)
```

Same character, two unrelated operators. No shared meaning to unify — the position is doing 100% of the disambiguation work.

**Rule: `const`, disambiguated by which of three slots it occupies**

```cpp
void f(const Point& p);   // inside parameter type → promise on the argument
int getX() const;          // trailing after parameter list → promise on *this
const Point p(1,2);        // on the declared variable → promise on p itself
```

Three slots, three different bindings, one word. You already hit this exact collision with `operator[]` needing const and non-const versions — that's this rule forcing your hand, not a special case.

**Rule: `operator[]`, definition-site vs call-site**

```cpp
int& operator[](int i);   // definition — inside a class body
v[1]                       // call — always desugars to v.operator[](1)
```

The call-site form never looks like a function call syntactically, but the grammar guarantees it rewrites to one. This is the same rule as `a + b` rewriting to `a.operator+(b)` or `operator+(a,b)` — operators are ordinary function-call grammar wearing a symbol as a disguise.

**The single move underneath all five rules:** when you hit an ambiguous token, don't ask "what does this symbol mean" — ask "what sits immediately around it, and which of the fixed positions does that put it in." The token is never the unit of meaning. The slot is.

Want the next layer — how the compiler actually resolves overload sets when multiple rules could match the same call (e.g. `operator+` member vs free-function candidates both in scope) — or is this layer solid first?
