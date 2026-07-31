## Geometry

[![Ubuntu Build](https://github.com/timow-gh/Geometry/actions/workflows/ubuntu-build.yml/badge.svg)](https://github.com/timow-gh/Geometry/actions/workflows/ubuntu-build.yml)
[![Windows Build](https://github.com/timow-gh/Geometry/actions/workflows/windows-build.yml/badge.svg)](https://github.com/timow-gh/Geometry/actions/workflows/windows-build.yml)

## Building the examples

The GeoQik examples use optional GUI and runtime dependencies that are not
needed by the Geometry library or its tests. Consequently, the regular CMake
presets set `Geometry_EXAMPLES=OFF`, while the dedicated examples presets:

- set `Geometry_EXAMPLES=ON`;
- enable the `geoqik-examples` vcpkg manifest feature; and
- configure through the vcpkg toolchain using the `x64-windows-static`
  triplet.

Keeping these configurations separate avoids installing the example-only
dependencies for normal library builds. It also requires a separate configure
step because vcpkg manifest features and the toolchain are selected when CMake
creates a build tree. The build and workflow presets are convenience wrappers
around that examples configuration.

Set `VCPKG_ROOT` to your vcpkg installation, then build the Debug examples on
Windows with:

```powershell
cmake --preset conf-msvc-debug-examples
cmake --build --preset build-msvc-debug-examples
```

Alternatively, configure, build, and run the tests in one step:

```powershell
cmake --workflow --preset workflow-test-msvc-debug-examples
```

Equivalent `release-examples` presets are available for Release builds.
