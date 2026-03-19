# Dkyb CMake Common

Reusable CMake settings for dkyb C++ repositories.

## What it standardizes

- Build types: `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`, `Coverage`, `Performance`
- Default build type: `RelWithDebInfo`
- Output directories:
  - `${binary_dir}/${config}/bin`
  - `${binary_dir}/${config}/lib`
- Language defaults:
  - `CMAKE_CXX_STANDARD=23`
  - `CMAKE_CXX_EXTENSIONS=OFF`
  - `CMAKE_EXPORT_COMPILE_COMMANDS=ON`
- Baseline flags:
  - Common: `-rdynamic -Wall -Werror`
  - Coverage: gcov/coverage flags
  - Performance: `-O3 -DNDEBUG -march=native -flto`

## Consumer usage

### Option A: Git submodule (most reproducible)

```bash
git submodule add https://github.com/kingkybel/cmake-common.git cmake-common
git submodule update --init --recursive
```

Then in your root `CMakeLists.txt`:

```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-common/cmake/DkybBuildSettings.cmake)
dkyb_apply_common_settings()
```

### Option B: FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    dkyb_cmake_common
    GIT_REPOSITORY https://github.com/kingkybel/cmake-common.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(dkyb_cmake_common)
include(${dkyb_cmake_common_SOURCE_DIR}/cmake/DkybBuildSettings.cmake)
dkyb_apply_common_settings()
```

Pin to a tag for stable/reproducible builds.
