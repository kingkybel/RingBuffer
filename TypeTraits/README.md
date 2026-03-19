![TypeTraits Banner](assets/banners/type_traits_banner.svg)

# TypeTraits

Header-only C++ type-trait helpers for compile-time introspection.

## Purpose

This repository provides reusable trait utilities to:

- detect whether operators, hashes, and member/static member functions exist with specific signatures
- classify string-like types (`std::basic_string`, `std::basic_string_view`, character pointers, and character arrays)
- reason about type compatibility at compile time (including character-type compatibility)
- support generic code paths that depend on string/character semantics

# Installation

The installation and build is tested on *ubuntu24.04 LTS*

## dependencies

googletest:

```bash
# create a directory where you like to clone googletest, eg: ~/Repos and change to it
mkdir ~/Repos ; cd ~/Repos
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
mkdir build
cmake ..
make -j $(nproc)
sudo make install
```

## use cmake to install the header-only library

```bash
# change the next line to change the install prefix to your liking
INSTALL_PREFIX=/usr
mkdir ./build
cd build
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
cmake --build . --parallel $(nproc)
sudo cmake --install .
```
This will install the headers from the include-folder to `${INSTALL_PREFIX}/dkyb`

To use the headers in your code, make sure that ${INSTALL_PREFIX} is in the include directories of your project.
Include the file in your code e.g:
```c++
#include <dkyb/traits.h>
```

## Powered by
Reduce the smells, keep on top of code-quality. Sonar Qube is run on every push to the `main` branch on GitHub.

[![SonarQubeCloud](assets/icons/logo-sonarqube-cloud-small.png)](https://sonarcloud.io/project/overview?id=kingkybel_TypeTraits)
