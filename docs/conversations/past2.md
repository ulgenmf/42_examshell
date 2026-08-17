Compiler generates these automatically IF you write none of them. Moment you write ANY constructor, default constructor generation stops (must write explicit C() yourself if still needed). Copy ctor / copy assign / dtor generation is separate — compiler still auto-generates those unless you define them yourself.

Default constructor: runs when object built with no args (bigint c;). Zero-init or explicit-init members here. If you hold raw pointer/array, must init to safe state (nullptr, 0-length) or dtor segfaults on delete of garbage.

Copy constructor: runs when new object built FROM existing one — bigint e(d);, or pass-by-value, or return-by-value (pre-C++17, RVO permitting). Signature must take const C& — reference to avoid infinite recursion (pass by value would itself need copy ctor to make the parameter), const because copying shouldn't mutate source.
Compiler default version: memberwise copy (shallow). Fine if members are primitives/other objects with correct copy ctors of their own. Breaks if you hold raw pointer to heap memory — shallow copy copies the pointer value, not what it points to. Two objects then hold same pointer, both destruct it, double free.

Copy assignment operator: runs on existing = existing2 (both already constructed). NOT same code path as copy ctor (which builds a new object). Must:

Check self-assignment (if (this == &other) return *this;) — matters if you free-then-copy internal resource; without check, freeing own resource before reading from other (which IS this) corrupts source before copy happens.
Return C& (reference to self) — enables chaining a = b = c (assignment right-associative, b=c must yield something assignable-from for next =).
Actually copy state from other into *this.

Destructor: runs on scope exit / delete. Releases owned resources (heap memory, file handles). virtual destructor mandatory ONLY when class used polymorphically (base class pointer deleting derived object) — this applies to polyset's bag base class, since you'll hold bag\* pointing at array_bag/tree_bag instances and delete through base pointer. Without virtual ~bag(), delete through base pointer calls only base dtor, derived resources leak.

None of your three subjects obviously need raw-pointer resource management (bigint likely uses std::string or std::vector internally, which self-manage) — but compiler-generated OCF members only correct if EVERY member's own OCF is correct. If you use raw array (char digits[N] fixed size, no heap) — compiler default OCF is fine, no need to write any of it. Write explicit versions ONLY when member is raw pointer to heap-allocated data, or class needs virtual dtor for polymorphism.

Operator overloading mechanism

a + b where a is class type: compiler looks for operator+. Two possible forms:

cpp
// member form — implicit first operand is \*this
C C::operator+(const C& rhs) const { ... }
// call: a + b == a.operator+(b)

// free function form — both operands explicit
C operator+(const C& lhs, const C& rhs) { ... }
// call: a + b == operator+(a, b)

Rule for which form to use: if left operand type is NOT your class (or must remain unmodified/no access to private members through member syntax), must use free function. This is why operator<< for printing is ALWAYS free function:

cpp
std::ostream& operator<<(std::ostream& os, const bigint& b) { ... }

std::cout << a desugars to operator<<(std::cout, a). Left operand is std::ostream, you don't own that class, can't add member function to it. Must be free function. If it needs access to private members of bigint, declare friend inside bigint:

cpp
friend std::ostream& operator<<(std::ostream&, const bigint&);

Compound assignment operators (+=, <<=, >>=): member functions, mutate \*this, return C& (reference, for chaining, same reasoning as operator=):

cpp
bigint& bigint::operator+=(const bigint& rhs) {
// mutate self
return \*this;
}

Plain binary operators (+, -): should NOT mutate operands, return new object by value. Commonly implemented in terms of compound version:

cpp
bigint bigint::operator+(const bigint& rhs) const {
bigint result(\*this);
result += rhs;
return result;
}

const at end = method doesn't mutate \*this. Required because subject calls a + b where a is const bigint. Without const qualifier on method, compiler refuses to call non-const method on const object — const-correctness enforced at compile time, method's const-ness is part of its signature, overload resolution checks it.

Prefix vs postfix increment/decrement — distinguished by dummy int parameter, not used, exists ONLY to give compiler distinct signature to pick:

cpp
C& operator++(); // prefix: ++b
C operator++(int); // postfix: b++

Mechanism difference:

prefix: mutate self, return reference to already-mutated self. One object touched.
postfix: must return value BEFORE mutation (since b++ evaluates to old value). Implementation:
cpp
C C::operator++(int) {
C tmp(*this); // save old state
++(*this); // mutate self via prefix
return tmp; // return old (copy)
}

Postfix always costs one extra copy construction. This is real cost, not just style — subject's v5++ followed by ++v5 exercises exactly this: postfix returns old value by copy, prefix returns new value by reference.

Comparison operators (<, <=, >, >=, ==, !=): each independent function, C++98 has no operator synthesis (<=> spaceship is C++20, not available in 42 exam environment). Must write all six separately, or write < and == then implement rest in terms of those:

cpp
bool operator<(const C& lhs, const C& rhs) { ... } // real comparison
bool operator==(const C& lhs, const C& rhs) { ... } // real comparison
bool operator>(const C& lhs, const C& rhs) { return rhs < lhs; }
bool operator<=(const C& lhs, const C& rhs) { return !(rhs < lhs); }
bool operator>=(const C& lhs, const C& rhs) { return !(lhs < rhs); }
bool operator!=(const C& lhs, const C& rhs) { return !(lhs == rhs); }

operator[]: member only (can't be free function — needs to return reference to internal storage for both read and write use: v2[1] = 12; requires non-const version returning int&; const version returning int or const int& for read on const object). Need BOTH overloads:

cpp
int& operator[](int i); // non-const object, mutable access
const int& operator[](int i) const; // const object, read-only access
Const correctness — full mechanism

const on object: const bigint a(43); — compiler tracks this at type level. Only methods marked const (after parameter list) callable on it. const method promises this pointer is const C\* const this internally — can't mutate any member through it (unless member itself marked mutable, rare, not needed here).

Why compiler enforces: prevents accidental mutation, and enables safe pass-by-const& (guarantees caller function won't modify your object through the reference — this is exactly what copy ctor / operator+ parameters const C& promise). If you pass const bigint& into a function and that function calls a non-const method on it, compile error — this is the compiler protecting the const promise.

Polyset — abstract class / virtual dispatch mechanism
bag (abstract)
├── searchable_bag (abstract, adds search interface)
│ ├── array_bag
│ │ └── searchable_array_bag (inherits array_bag, implements searchable_bag)
│ └── tree_bag
│ └── searchable_tree_bag (inherits tree_bag, implements searchable_bag)

Pure virtual function: virtual void insert(int x) = 0; — = 0 means no implementation in this class, AND compiler blocks instantiation of this class (bag b; = compile error). Any class inheriting from bag remains abstract UNTIL every pure virtual is overridden with real implementation. searchable_array_bag must override every pure virtual from both array_bag's ancestry and searchable_bag's interface, or it stays abstract too, instantiation still blocked.

Mechanism behind dispatch: each polymorphic class instance carries hidden pointer, vptr, pointing at class's vtable (array of function pointers, one per virtual function, built once per class at compile time). Call through base pointer:

bag* b = new searchable_array_bag();
b->insert(5);
b (type bag*)
│
▼
[ searchable_array_bag object ]
vptr ──► [ vtable for searchable_array_bag ]
insert → searchable_array_bag::insert
search → searchable_array_bag::search
~bag() → searchable_array_bag::~searchable_array_bag

Call b->insert(5) doesn't jump straight to bag::insert — runtime reads vptr from actual object, indexes into vtable, jumps to whichever override actually exists on the CONCRETE type. This is why base class dtor must be virtual — delete b; looks up vtable too, must find derived dtor, or derived-only members never get released.

Non-virtual function call, by contrast, resolved at compile time purely from static type of pointer/reference — no vtable lookup, always calls base version regardless of actual object type. Only functions marked virtual get this runtime dispatch.

set class wraps a searchable_bag* (likely holds pointer or reference to abstract type, decided by ctor argument — allows set to work with either array_bag-backed or tree_bag-backed storage interchangeably, since caller passes concrete searchable_array_bag or searchable_tree_bag, set only ever sees it through searchable_bag&/* interface). This is the entire point of the exercise — polymorphism lets set be storage-agnostic.

bigint — subject-specific mechanism

Storage: not specified which container — commonly implemented as std::string of digit characters (base 11, so digits '0'-'9' plus one more symbol for digit value 10 — subject doesn't state symbol, check attached files/defaults, often 'A' or similar used in 42 base-11 bigint subjects) or std::vector<unsigned> storing digit values 0-10 directly. Vector of digit-values simpler (no char-to-value mapping needed for arithmetic), string simpler for direct printing.

digitshift (<<, >>): NOT bitshift. Shift entire digit positions in base 11 representation. 43 << 3 == 42000 — decimal 43 (or bigint holding value 43) shifted left 3 digit-positions in base-11 means append 3 zero-digits at least-significant end (equivalent to multiply by 11³). Note stated example uses decimal-looking numbers but operation is defined in base 11 internally — value 43 stored as base-11 digits, shifted, printed back as base 11. Mechanism: shifting is literally moving elements in your digit-array by N positions (like array shift, not multiplication algorithm) — much cheaper than repeated multiplication, that's the point of the exercise (exercise arbitrary precision shift op separate from arithmetic op).

Right shift >>: remove N least-significant digits (integer division by 11^N, discard remainder) — 1337 >> 2 == 13 in base 11 interpretation.

No leading zeros on print: after any operation that could produce leading zero digits (subtraction not required per subject, but shift/add could theoretically not, still worth checking) — before writing digit array to ostream, skip all leading '0' digit entries until first nonzero or until only one digit left (must print "0" itself if bigint value is zero, not empty string).

Addition with carry: standard schoolbook algorithm — sum digit-pairs from least-significant end, track carry, base is 11 not 10:

cpp
carry = 0
for i from least-significant to most-significant:
sum = digit_a[i] + digit_b[i] + carry
result[i] = sum % 11
carry = sum / 11
if carry > 0: append carry as new most-significant digit
vect3 — subject-specific mechanism

Dimension 2 (despite name vect3 — subject text itself is inconsistent, "vect3.hpp"/"vect2.hpp" both appear, main uses v2[0], v2[1], two components). Two int members, likely named arbitrarily (x*,y* or arr[2]).

Scalar multiplication needs BOTH directions to satisfy vect3(2,2) _ 2 and 3 _ v2 (seen in main):

cpp
vect3 operator*(const vect3& v, int scalar); // v * scalar, could be member instead
vect3 operator*(int scalar, const vect3& v); // scalar * v — MUST be free function, left operand is int, you can't add member to int

If first one is member (vect3::operator\*(int) const), second one MUST still be free function delegating to it:

cpp
vect3 operator*(int scalar, const vect3& v) { return v * scalar; }

operator[] bound access — subject explicitly states "no bound checking required", meaning skip range check most containers would need — direct array/member index return, operator[](int i){ return i == 0 ? x* : y*; } or backed by int comp[2] array directly indexed.

Unary minus (-v2 used in main) separate from binary minus:

cpp
vect3 operator-() const; // unary, negate both components
vect3 operator-(const vect3&) const; // binary, subtract

Both same operator symbol, disambiguated by parameter count (0 vs 1 explicit param, aside from implicit this).

Ask about any piece here to go deeper — vtable layout detail, digit-array carry edge cases, OCF self-assignment failure modes, whatever's shaky.

Uh, just understand that, uh, it has to be, uh, nine... ninety eight... CPP ninety eight. So don't use newer features. I know the vector two, uh, that one I understand, but I will need... do do we need to do? But the others, uh, first, what we threw the other two, like, what are we... are we being asked to do exactly? What are we need to

Deciphered user's C++98 constraints and clarified requirements

C++98 only. No auto, no nullptr (use NULL), no range-for, no override/final, no <=>, no smart pointers, no lambda, no >> template-closing without space (need vector<vector<int> > style if nested templates appear, though unlikely here).

bigint — task exact

Store arbitrary-precision unsigned int. No sign handling needed.

Required ops (from subject text):

addition (+, and += since main uses c += a)
comparison — main exercises all six: <, <=, >, >=, ==, !=
digitshift << / >> — shift digit positions base 11, not bits. 43 << 3 == 42000, 1337 >> 2 == 13. Also <<=, >>= since main uses d <<= 4, d >>= (const bigint)2.
print via operator<< (free function, base 11, no leading zeros)

Main also forces (not in prose spec, but main is contract too):

default ctor: bigint c;
copy ctor: bigint e(d);
construct from int: bigint a(43);
prefix ++b, postfix b++

Prefix/postfix — two separate functions, distinguished by dummy unused int param on postfix signature. Prefix returns &, postfix returns copy of pre-mutation state.

> > = takes const bigint (temporary cast in main: (const bigint)2), meaning need implicit-or-explicit ctor from int for that cast to work, non-explicit likely (main relies on it).

polyset — task exact

Given (unmodified) files: bag (abstract, base ops — insert/remove/etc, no search), searchable_bag (abstract, adds search interface on top of bag), array_bag (concrete bag, array-backed, no search), tree_bag (concrete bag, BST-backed, no search).

Part 1 — you write:

searchable_array_bag : inherit array_bag (get array storage + base bag ops) AND implement searchable_bag's pure virtual search interface (fulfill contract, make class concrete/instantiable)
searchable_tree_bag : same pattern, inherit tree_bag, implement searchable_bag

Job here: write only the search-related methods (the ones searchable_bag declares pure virtual, array_bag/tree_bag don't already provide) — using whatever underlying storage array_bag/tree_bag already expose (protected members, or via base class's own accessors) to actually do the search.

Part 2 — you write:

set class. Wraps one searchable_bag (by pointer or reference — polymorphic, works with either concrete type above interchangeably). Job: enforce set semantics (no duplicate elements) on top of bag semantics (bag = duplicates allowed, like a multiset). Insert into set = check searchable_bag's search first, only insert into underlying bag if not already present.

All 5 classes (searchable_array_bag, searchable_tree_bag, set, + whatever's implied) — Orthodox Canonical Form required, const-correct.

Given main for polyset not shown in your paste — if you

Claude’s response was interrupted.
class

Type bundling data (members) and functions (methods) that operate on that data together, one unit.

cpp
class Point {
private:
int x;
int y;
public:
int getX() const { return x; }
};

private: only this class's own methods can touch these members directly. public: accessible from anywhere. protected: this class + any class inheriting from it can touch, outside code can't. Access control checked entirely at compile time — pick wrong access level, compile error, not runtime failure.

Method defined outside class body uses :: (scope resolution operator), states which class this function belongs to:

cpp
// declared in header:
class Point { public: int getX() const; };
// defined in source:
int Point::getX() const { return x; }
this

Inside any non-static method, hidden pointer to the object the method got called on. x inside a method body means this->x, implicitly. Used explicitly for: comparing identity (this == &other, checking self-assignment), returning reference to self (return \*this;, enables operator chaining).

Constructor

Function that runs automatically the moment object comes into existence. Never called manually like obj.construct() — happens as part of the object's creation syntax itself.

cpp
class Point {
public:
Point(); // default constructor — runs on `Point p;`
Point(int x, int y); // parameterized — runs on `Point p(1, 2);`
Point(const Point& other); // copy constructor — runs on `Point p2(p);` or pass-by-value or return-by-value
};

If you write zero constructors, compiler generates default one automatically. Moment you write ANY constructor yourself, compiler stops auto-generating the default one — if you still need Point p; to work, must write Point(); yourself explicitly.

Copy constructor takes const Point& — reference (not value) so it doesn't need to copy the argument to receive it (which would need copy constructor to make its own parameter — infinite recursion), const so copying doesn't mutate the thing being copied from.

Destructor

Function running automatically when object's lifetime ends (scope exit for stack objects, delete for heap objects). One per class, no parameters, name is ~ClassName().

cpp
class Point {
public:
~Point() { }
};

Compiler auto-generates one if you write none. Matters most when class owns a resource needing explicit release — irrelevant for your three subjects if using self-managing containers like std::vector/std::string internally.

virtual destructor — needed only when object gets deleted through pointer to its base class (bag\* b = new array_bag(); delete b;). Without virtual on base's destructor, delete b calls only base class's destructor, derived-class-only members/resources never get cleaned up.

Copy assignment operator

Runs when an already-existing object gets assigned a value from another already-existing object. Different situation from copy constructor (which builds a brand new object) — this one fires on existingA = existingB;.

cpp
Point& operator=(const Point& other) {
if (this == &other) return *this; // self-assignment guard
x = other.x;
y = other.y;
return *this;
}

Self-assignment guard matters when assignment involves releasing old internal state before copying new state in — without the check, a = a would release/corrupt state before reading from itself. Returns Point& (reference to self) specifically to allow chaining: a = b = c — this requires b = c to itself produce something assignable into a.

Orthodox Canonical Form

Term for the bundle: default constructor + copy constructor + copy assignment operator + destructor. Every class in your three subjects needs all four written or correctly compiler-defaulted.

const

Three distinct positions it appears, three distinct meanings:

cpp
void f(const Point& p); // p: caller's object passed in, this function promises not to mutate it through p
int getX() const; // this method promises not to mutate the object it's called on
const Point p(1, 2); // p itself: cannot be mutated ever again after this line, by anyone

Object marked const can only call methods themselves marked const. Enforced at compile time — calling a non-const method on a const object is a compile error, full stop, checked before program ever runs. This is why methods that only read (never mutate) — getters, comparison operators, printing — must carry the const qualifier, or they become uncallable on any const object.

Reference (& in a type)

Alternate name for an existing variable, not a separate object.

cpp
int x = 5;
int& r = x; // r IS x, second name for same memory location
r = 10; // x is now 10 too

Must be bound to something at the moment it's declared — cannot exist unbound, cannot later be repointed at a different variable, stays tied to whatever it was bound to at creation forever.

Function parameters using const ClassName& — passing the caller's actual object in, no copy made (cheap), but caller's object protected from mutation by const (safe). This is the default parameter pattern across your three subjects — nearly every function taking another instance of the same class as argument uses this exact form.

Function overloading

Same function name, multiple versions, distinguished by parameter types/count. Compiler picks which version to call based on what you pass at the call site.

cpp
void f(int x);
void f(double x);
f(5); // calls int version
f(5.5); // calls double version
Operator overloading

Special case of function overloading — operators like +, <<, [], == are just functions with reserved names (operator+, operator<<, etc) that the compiler connects to the symbol automatically.

Two forms:

cpp
// member — implicit left operand is \*this
Point Point::operator+(const Point& rhs) const { ... }
// a + b desugars to: a.operator+(b)

// free function — both operands explicit
Point operator+(const Point& lhs, const Point& rhs) { ... }
// a + b desugars to: operator+(a, b)

Must use free function form whenever left operand isn't your class type — this is why printing (std::cout << obj) is always a free function: left operand is std::ostream, a type you don't own, can't attach a member function to it.

If a free function operator needs access to your class's private members, declare it friend inside the class:

cpp
class Point {
friend std::ostream& operator<<(std::ostream&, const Point&);
};

Compound assignment operators (+=, <<=) — always member, always mutate \*this, always return ClassName& (same chaining reasoning as operator=).

Plain binary operators (+, -) — should not mutate either operand, return new object by value, commonly implemented by copying self then calling the compound version:

cpp
Point Point::operator+(const Point& rhs) const {
Point result(\*this);
result += rhs;
return result;
}

Prefix vs postfix increment — same operator symbol, disambiguated by a dummy unused int parameter on postfix only:

cpp
Point& operator++(); // prefix: ++p — mutate self, return reference to self
Point operator++(int); // postfix: p++ — must return value from BEFORE mutation
cpp
Point Point::operator++(int) {
Point tmp(*this); // save state before mutation
++(*this); // mutate self via prefix version
return tmp; // hand back old state, by value
}

operator[] — needs two overloads, non-const returning a mutable reference (allows obj[i] = x;), const returning read-only access (for use on const objects):

cpp
int& operator[](int i);
const int& operator[](int i) const;
Inheritance
cpp
class Base {
public:
virtual void speak() { }
};
class Derived : public Base {
public:
void speak() { } // overrides Base's version
};

Derived automatically has everything Base has, plus whatever Derived adds itself. public before Base — access levels carry through unchanged (a Base public member stays public in Derived).

virtual / pure virtual / abstract class

virtual on a method — marks it as overridable, and more importantly, callable through a base-class pointer/reference while still running the derived class's actual version (this is called dynamic dispatch, resolved at runtime, not compile time).

cpp
Base* b = new Derived();
b->speak(); // calls Derived::speak(), even though b's static type is Base*

Mechanism: every object of a class with virtual methods carries a hidden pointer (vptr) to a table of function pointers (vtable), one table per class, built once. Calling a virtual method through a pointer/reference reads this table at runtime, jumps to whichever version the actual underlying object has.

Pure virtual — virtual void speak() = 0; — declares the method exists but provides no body in this class, AND makes this class impossible to instantiate directly (Base b; becomes a compile error). Called an abstract class. Any class inheriting from it remains abstract itself until every pure virtual method has been given a real implementation somewhere in the inheritance chain — only then can it be instantiated.

Ask about any single piece to go deeper, or point at which of the three subjects to map this onto next.
