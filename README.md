# poly-physx-demo

poly-physx-demo is a visual demo application showcasing the capabilities of the [poly-physx](https://github.com/ismawno/poly-physx) engine. The engine is designed to simulate any convex polygonal physical objects in 2D environments. This demo is a representation of poly-physx's features and capabilities, aiming to provide an easy-to-understand and interactive demonstration of the physics engine.

## Features

- Convex polygon collisions
- Springs and user-defined forces
- Customizable physics constraints
- Rigid bars (joints) between polygons
- Cross-platform compatibility (MacOS and Windows)

## Dependencies

All dependencies are included as git submodules in the repository. Clone the repository recursively to download all submodules: `git clone --recursive https://github.com/ismawno/poly-physx-demo.git`. No extra steps are required to install the dependencies, except for cloning the repository recursively.

The following git submodules are included:

- [poly-physx-app](https://github.com/ismawno/poly-physx-app)
- [poly-physx](https://github.com/ismawno/poly-physx)
- [SFML](https://github.com/ismawno/SFML)
- [imgui](https://github.com/ismawno/imgui)
- [imgui-sfml](https://github.com/ismawno/imgui-sfml)
- [implot](https://github.com/ismawno/implot)
- [glm](https://github.com/g-truc/glm)
- [sfml-primitives](https://github.com/ismawno/sfml-primitives)
- [rk-integrator](https://github.com/ismawno/rk-integrator)
- [allocators](https://github.com/ismawno/allocators)
- [debug-log-tools](https://github.com/ismawno/debug-log-tools)
- [spdlog](https://github.com/gabime/spdlog)
- [profile-tools](https://github.com/ismawno/profile-tools)
- [yaml-cpp](https://github.com/ismawno/yaml-cpp)
- [container-view](https://github.com/ismawno/container-view)
- [geometry](https://github.com/ismawno/geometry)

## Setup

### MacOS

Run the [setup-macos.sh](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-macos.sh) script. This will automatically install the necessary software: XCode Command Line Tools, homebrew, python (3.5 of later), CMake, premake, and make. If authorized, it will also generate the project build files using gmake, the only supported generator on this platform.

### Windows

Run the [setup-win-vs.bat](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-win-vs.bat) or the [setup-win-mingw.bat](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-win-mingw.bat) script, depending on your desired generator. This will automatically install the necessary software and generate the project build files for such generator.

### Generating Build Files

It is also possible to anually execute the [generate_build_files.py](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/generate_build_files.py) script. This script is responsible for compiling the [SFML](https://github.com/SFML/SFML) library and generate the project build files for the specified generator. The script is executed the same way for both operating systems, with the default generator changing depending on the platform. For Windows, if gmake is chosen as the generator, the MinGW compiler will be used, and the script will automatically install the software if not found. Run `python generate_build_files.py -h` to see a list of available commands.

### Compilation

#### MacOS

From the root, simply run the `make` command, and the project will compile with the default configuration. Type `make help` for a list of configurations.

#### Windows

For Visual Studio, open the generated solution and compile the project from there. Visual Studio should handle the rest
For gmake with MinGW, from the root, simply run the `mingw32-make` command, and the project will compile with the default configuration. Type `mingw32-make help` for a list of configurations.

## License

poly-physx-demo is licensed under the MIT License. See LICENSE for more information.