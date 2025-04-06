# C++ Coding Style

Loosely based on [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
A quick reference is below.
Add to this document if you look for something and realize it's not here.

## Naming Conventions

**FILES**:
- Use `.cc` for source and `.h` for headers.
- name files using underscores and their test files using `filename_test.cc`.
E.g., `my_class.cc`, `my_header.h`, `foo_bar.cc`.

**Types (classes, structs, type aliases, enums, type template params)**:
PascalCase with no underscores or spaces.
E.g., `class UrlTable`, `PropertiesMap`.

**Variable names**:
Use snake_case.
E.g., `table_name`, `my_var`.
Data members of classes have a trailing underscore. Both static and non-static.
E.g., `class_member_`, `class_id_`.

**Constant Names**:
Lead with `k` and use mixed case;
E.g., `kMySocket`, `kPi`, `kMyConstant`.
(Because CAP_CASE is for macros).

**Function Names**:
Use camelCase.
E.g., `myFunc()`, `foo()`.

## Includes
Including files into your source should always be in the following order:

```cpp
#include "foo.h"    // the corresponding header if any

#include <iostream> // stdlib
#include <vector>

#include <json> // third-party

#include "auth.h" // project headers
```

## Guards
Do not use `#pragma once` use the guards instead.
Guards should always have the form `<PROJECT>_<PATH>_<FILE>_H_`

For example, our dispatcher:
```cpp
#ifndef FOLSERV_DISPATCHER_H_
#define FOLSERV_DISPATCHER_H_
// ...
#endif // FOLSERV_DISPATCHER_H_
```

## Comments

**Header files**:
Header files must have a file comment describing their purpose and their scope.
```cpp
/**
 * example_file_name.h
 * 
 * This file does x, y, z.
 * Use when...
 * Tips + extra information here.
 * /
```

**Functions**:
Functions have to have doxygen comments in the header file but not necessarily in the source file.
```cpp
/// @brief Function Description
/// @throws std::runtime_error because...
/// @param x some description
/// @return x + 1
int myFunc(int x);
```

## Organization

### Header and Source
Put header and source files in the same directory.

## Namespaces
Do not use `using namespace std;`. 
Import what you use and namespace it properly.

## Classes vs. Structs
Use structs if all or the vast majority of member vars are public, class otherwise.