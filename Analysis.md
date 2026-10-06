# Analysis

1. Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?<br>  
    - **String**  
    Java implements strings by building an immutable `String` object in the case of declaration of a string via a string literal. This helps save memory by storing the string in a String constant pool, allowing variables to simply point to the same reference. It helps when we are reusing the same string for different use cases, but becomes a problem when we create a lot of different strings. The string pool also needs to undergo garbage collection whenever the string constant stops being used, which could slow down the speed when there is constant reassignment of string variables.  
    Although we can't reuse string constants for similar strings, the implementation we have allows us to dynamically allocate and reallocate memory for strings allowing them to be mutable as well as letting us easily expand and concatenate without leaving dead memory.
    - **Map**  
    Java's `HashMap` creates an array of nodes, each node contains a `hash` integer, a key `K`, and a value `V`, and then finally a `next` property pointing to the next node in the bucket chain as it utilizes *closed addressing*.  
    The primary difference between Java's hashmap and this implementation is the lack of insertion order keeping in Java's `HashMap`. Without insertion ordering, it is much more difficult to easily print out a list of keys in the map without having to traverse the entire array.
2. You wrote the tag check in `dt_value_as_int` by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's `enum` and `match` work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?<br>

    - Our `dt_value` exposes its tag and union payload separately. `dt_value_as_int` checks that the tag is `DT_INT` before reading the integer payload, but another function can bypass it and access v.as.integer directly. C also lets a programmer assign a tag that does not describe the stored payload. The compiler does not enforce the relationship between these fields. Reading the wrong union member can reinterpret its stored representation and produce a meaningless value. Depending on the representation and later use, it can lead to undefined behavior. For example, treating an integer’s bits as a string pointer and dereferencing that pointer is dangerous. 

    Safe Rust instead associates the payload with an enum variant. Pattern matching identifies the variant before exposing its data, and a match must cover every possibility. A wildcard counts as coverage, so exhaustiveness does not require a separate meaningful action for every variant.

    C’s freedom can be useful when working with existing C interfaces, controlling representation, or avoiding a repeated check after another operation has already established the type. However, unchecked access is not automatically faster: a compiler can eliminate redundant checks in a language that enforces safety.
    
3. Your `dt_map` keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.<br>  
Removing insertion order makes the output less difficult which will hinder the testing process. It also makes it harder to keep track of the history should you need to display it. Personally I think keeping insertion orders is a necessary step when the use needs it, and for this one we want to test our map behvaior so I would not ship something with no insertion order.


4. Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?<br>

    - Access after release can read reused memory, corrupt data, or crash. Our release flag prevents borrowing the freed cell while its handle remains valid.
    
    Unreleased allocations instead accumulate. In a server, repeated retention can cause memory pressure and allocation failures. In a short command-line tool, process exit normally reclaims memory, reducing the lasting cost; stale access can still cause damage immediately.

    Our driver reports unreleased references before cleanup, then destroys their cells and handles. Its leak error therefore detects a release-contract violation, even though final cleanup frees the memory.
