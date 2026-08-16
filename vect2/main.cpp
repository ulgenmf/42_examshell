Not this: "RAII handles cleanup for you automatically."
This instead: "RAII: a resource is acquired in a constructor and released in the
matching destructor. C++ guarantees a destructor runs when an object's scope
ends, including during stack unwinding from an exception. Tying a resource's
lifetime to an object's lifetime means it's released exactly once, exactly when
the object stops existing, with no path that skips it."

