# AGENTS.md

> **For coding agents working on this repository:**
> This guide contains essential information on building, code style, structure, commands, and best practices for effective, safe contribution to this chess game project.

---

## 1. Project Overview

- **Project type:** C++11 Chess game using [ncurses](https://invisible-island.net/ncurses/announce.html)
- **Platform:** Linux (focus)
- **Build System:** [CMake](https://cmake.org/) + [Make](https://www.gnu.org/software/make/)

## 2. Directory Structure

- `/src/`: C++ source files (`main.cpp`, `piece.cpp`, `game.cpp`, `ai_player.cpp`)
- `/include/`: C++ headers (`game.h`, `piece.h`, `ai_player.h`)
- `/build/`: Created by build steps, not under version control
- `/img/`: Images for README visuals
- `CMakeLists.txt`: Project build configuration
- `README.md`: Brief build/run info and screenshot

## 3. Build, Lint, and Test Commands

### 3.1 Building

```bash
mkdir -p build
cd build
cmake ..
make
```
- Produces `build/Chess` executable
- No external scripts; see `CMakeLists.txt` for settings

### 3.2 Running

```bash
./chess
```
(Run from inside `build/`)

### 3.3 Linting
- **No standard linter configured.**
- For new code, prefer to run `clang-tidy` or `cpplint` locally if modifying large portions. Add a linter config if applying format-wide fixes.

### 3.4 Testing
- **No automated/unit test framework is present.**
- Testing is manual: run the game and test features interactively.
- If contributing tests, place them under `tests/` (create if absent), use [Catch2](https://github.com/catchorg/Catch2) or [GoogleTest](https://github.com/google/googletest). Provide CLI for targeting single tests where possible in future.

---

## 4. Code Style Guidelines

### 4.1 File Layout
- One C++ class per header/source file
- All headers in `/include/`, includes use `#include "file.h"`
- Keep functions, classes, and constants in appropriate files
- Use `#pragma once` for header guards

### 4.2 Imports/Includes
- Place all includes at the top of each file
- Standard library before third-party before project headers
- Use angle brackets for system headers, quotation marks for local: 
  `#include <ncurses.h>` 
  `#include "game.h"`

### 4.3 Formatting
- Indentation: 4 spaces, **no tabs**
- Brace style: *Stroustrup* (`function foo()\n{ ... }`)
- One statement per line
- Max line length: 100 (soft)
- Wrap comments at 80 characters

### 4.4 Typing & Declarations
- Explicitly declare all types and return values
- Local variables as close to usage as possible
- Use `const` wherever possible
- Use modern C++ features from C++11 (range-for, auto, nullptr, etc.)
- Prefer `std::vector` and other STL containers

### 4.5 Naming Conventions
- **Classes:** PascalCase (e.g., `AIPlayer`, `Game`)
- **Functions:** camelCase (e.g., `makeMove`, `drawUi`)
- **Variables (public):** camelCase
- **Variables (private)/fields:** snake_case (e.g., `current_round`)
- **Constants/macros:** ALL_CAPS with underscores (e.g., `BOARD_SIZE`)
- **Files:** snake_case (but project is tolerant of different case)

### 4.6 Comments & Documentation
- Every class and major function should have a comment header
- Inline comments for tricky logic (English preferred, Chinese accepted; match file’s precedent)
- Avoid commented-out code in versioned files

### 4.7 Error Handling
- Prompt error feedback to end user: print error and exit when unrecoverable
- Check validity of `new` allocations where heap is used
- Check for errors from ncurses or file operations; clean up before exit
- Avoid exceptions; use error codes or booleans to signal recoverable failures

### 4.8 General Best Practices
- Keep functions short and focused
- Limit class scope as much as possible
- Declare destructors virtual only if inheritance is intended
- Make interfaces clear and hide implementation details
- Do not use magic numbers – prefer descriptive `#define` or `const` values
- Remove unused variables and commented code
- Write maintainable over clever code
- If adding new dependencies, update README

### 4.9 Internationalization
- Comments/strings can be in English or Chinese. For new code, prefer English for code/comments visible to users or contributors.

---

## 5. Special Rules
- **Cursor/Copilot rules:** No `.cursor/rules/` or `.github/copilot-instructions.md` found as of this writing.
- If such rules are added in the future, integrate them here.

---

## 6. Adding and Updating Tests or Tools
- If you add unit tests or linters, update this file with usage/provisions
- Recommended to use Catch2 or GoogleTest for C++
- Preferred linter: clang-tidy
- If adding CI, prefer GitHub Actions

---

## 7. Agent Hints and Cautions
- Always run the build from a fresh shell when editing CMake settings
- If adding third-party code or vendoring, use a subdirectory or mark clearly
- Never commit `build/` or any binaries
- Adhere to these guidelines for contributions—raise a note here if you need exceptions

---

Happy Hacking!
