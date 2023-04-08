# poly-physx-demo

poly-physx-demo is a full demo application that showcases the capabilities of the [poly-physx](https://github.com/ismawno/poly-physx) engine, utilizing the visualization features of [poly-physx-app](https://github.com/ismawno/poly-physx-app) as its base. The demo allows users to add entities, springs, rigid bars, forces, interactions, and more through an in-app UI. This is a complete application, not a library.

## Features

- Full demo application showcasing the [poly-physx](https://github.com/ismawno/poly-physx) engine's capabilities
- Uses [poly-physx-app](https://github.com/ismawno/poly-physx-app) visualization features as a base
- In-app UI for adding entities, springs, rigid bars, forces, interactions, and more
- Pre-configured premake5 workspace

## Dependencies

All dependencies for poly-physx-demo are included as git submodules in the repository. Make sure to clone the repository recursively to download all submodules: `git clone --recursive https://github.com/ismawno/poly-physx-demo.git`. The list of dependencies is the following:

- [poly-physx-app](https://github.com/ismawno/poly-physx-app): Visualization tools for the [poly-physx](https://github.com/ismawno/poly-physx) engine
- [poly-physx](https://github.com/ismawno/poly-physx): The original 2D physics engine
- [SFML](https://github.com/ismawno/SFML): The Simple and Fast Multimedia Library (build from source using CMake from the fork on the author's GitHub page)
- [Dear ImGui](https://github.com/ismawno/imgui): Immediate-mode graphical user interface library
- [imgui-sfml](https://github.com/ismawno/imgui-sfml): ImGui backend for SFML
- [vec-2D](https://github.com/ismawno/vec-2D): A 2D vector implementation with usual operations
- [shapes-2D](https://github.com/ismawno/shapes-2D): A polygon geometry library for creating and manipulating convex polygons
- [rk-integrator](https://github.com/ismawno/rk-integrator): an implementation of the Runge-Kutta method for integrating the movement of entities
- [debug-tools](https://github.com/ismawno/debug-tools): A set of tools for debugging poly-physx simulations
- [profile-tools](https://github.com/ismawno/profile-tools): A set of tools for profiling poly-physx simulations
- [ini-parser](https://github.com/ismawno/ini-parser): A simple INI file parser that allows for reading and writing the state of the simulation to and from a file
- [vector-view](https://github.com/ismawno/vector-view): A header only library for modifying the contents of a std::vector without letting the user to modify its size.
- [sfml-primitives](https://github.com/ismawno/sfml-primitives): A C++ library providing line primitive implementations

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

## Building and Usage

1. Clone the repository recursively to download all submodules.
2. Run the `generate_build_files.py` script located in the `scripts` folder. This script will automatically build SFML and generate the build files for the rest of the project. Note that `generate_build_files.py` currently supports MacOS, with Windows support in progress.
3. Compile the project using the `make` command and execute it. The distribution by default is release.

For more information on how to use poly-physx-demo and explore its features, please refer to the documentation.

## License

poly-physx-demo is licensed under the MIT License. See LICENSE for more information.
