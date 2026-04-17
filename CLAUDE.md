# hello-claude

## Comment Style

All source files use Doxygen-style documentation.

**File headers** — every `.h` and `.cpp` file opens with a `/** @file */` block
containing `@brief`, `@author` (Robert Lowe and Claude (Anthropic)), and
`@copyright` (MIT License, Copyright (c) 2026 Robert Lowe).

**Declarations** — every class, struct, enum, and public/protected member in a
header has a `/** @brief … */` doc comment; `@param` and `@return` tags are
added where applicable. Inline member variables use `///<` trailing comments.

**Definitions** — each function definition in a `.cpp` file is preceded by a
one-line `/// @brief …` comment.

**Inline comments** — use paragraph style: one comment line describes the
intent of each logical block of code, with a blank line separating paragraphs.
Comments explain *why*, not *what* — the code itself says what it does.

## Commit Style

Always include the following co-author trailer on every commit:

```
Co-authored-by: Robert Lowe <rlowe8@utm.edu>
```
