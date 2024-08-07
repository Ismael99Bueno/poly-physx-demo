# poly-physx-demo

poly-physx-demo is a visual demo app I use to visualize my 2D physics engine [poly-physx](https://github.com/ismawno/poly-physx) (see its README for more details). It allows me to test/debug new features I implement

Some notes about [poly-physx](https://github.com/ismawno/poly-physx) (this is contained in its own README file, but as this is the main repo I work on, I'll leave it over here as well)

poly-physx is a 2D physics engine that I started as a hobby project in September 2022. It attempts to implement some of the most prominent features a physics engine has, such as collision detection, constraint solving, island management, etc. When I started, I had no idea how a physics engine was designed (I didn't even know what broad/narrow phase meant). Because of that, I made some poor design choices that required me to rewrite many components of the engine throughout development. Even after all these rewrites, there are still many things that I didn't implement but would have been nice to have from the beginning, such as:

- Unit testing
- CI/CD with GitHub Actions
- Use of CMake (current build system I use is Premake)
- Implementing a versatile API, including 2D and 3D
- Allowing the use of double precision floating points
- Switching the coding style (too much snake_case)
- Multithreading support from the beginning

I also took the "make it so that I can reuse everything I can" approach, which is not inherently bad, but it makes the engine less portable and harder to integrate into other projects. The actual physics engine repository, [poly-physx](https://github.com/ismawno/poly-physx), as it stands, won't build. You actually have to set it up as a git submodule because I wanted to separate anything that didn't have to do strictly with physics into another project because "what if I need it later in another project". I figured git submodules were the best option, but damn, it is annoying. I also decided to use Premake instead of CMake. It definitely has its advantages, but I don't think it's worth it. CMake is much more widely used and has other features I want to test. All in all, making everything reusable is not as good as it seems, mainly because that "other project" most likely won't happen, and it limits my ability to optimize. I have also had countless times where I have thought "this would be incredibly easier if I didn't insist on making it project agnostic", so I am tired of that idea.

Most of the other features I have listed are very enjoyable to implement from the start but an extreme pain to integrate into an existing project that doesn't support them. Proper multithreading has been around for some time now, but it’s not in a state I am satisfied with, and I would like to make it more multi-core friendly. Because of all this, this project is now discontinued, and I will focus on developing a new physics engine, applying everything I've learned from the beginning.

Regarding performance, I would say it is mediocre compared to other physics engines available. Some of the latest tests I conducted on my M1 MacBook involved 4 rotating tumblers with 1000 bodies each (4000 in total), with roughly 4.5ms per step. The most prominent issue seems to be cache misses throughout the whole application, but the main bottleneck is constraint contact solving, especially due to memory indirection when iterating over a collection of contact pointers (block allocated, yes, but still not ideal). I was about to optimize that part by using tight structs with constraint information until I decided I wanted to start over, so I guess that won't get done.

Another thing that was left on the back burner was to stop using std::vector to store block-allocated objects such as bodies, colliders, or contacts, and use linked lists instead. As they are already block-allocated, I am not worried about the memory fragmentation linked lists can introduce. Plus, the actual objects would act as the nodes themselves, making insert/erase operations very fast. This is especially important when removing a contact from an island, where I had to traverse a std::vector to find the contact and then shift the entire vector. A single remove() call cost around 3-5 microseconds. In intense scenarios, this method could be called around 100-200 times, which is just too much.

I have read some posts about island management lately, and I have seen a focus on this area regarding island merging (which, contrary to splitting, has to happen immediately). The Union-Find algorithm seemed the better choice, and I was determined to implement it, but I noticed that my current merge implementation only took a couple of microseconds to complete in intense scenarios (which didn't even account for 0.1% of the performance bottleneck), so I abandoned the idea. What I ended up optimizing was the splitting, which could take a long time in some scenarios (only one island is allowed to split per step).

## Features

- Actions panel: Use the body editor to create bodies with different shapes, edit their properties, spawn new bodies etcetera
- Performance panel: Track the app's performance and export data to csv to be visualized with the benchmark scripts
- Physics panel: Enable/disable different force/interaction behaviours
- Scenarios panel: Run different scenarios, most of them used to benchmark the app
- Engine panel: Tweak the engine's collision, integration and constraint settings

## Dependencies

All dependencies are included as git submodules in the repository. Clone the repository recursively to download all submodules: `git clone --recursive https://github.com/ismawno/poly-physx-demo.git`. No extra steps are required to install the dependencies, except for cloning the repository recursively.

The following git submodules are included:

- [poly-physx-app](https://github.com/ismawno/poly-physx-app)
- [poly-physx](https://github.com/ismawno/poly-physx)
- [geometry](https://github.com/ismawno/geometry)
- [rk-integrator](https://github.com/ismawno/rk-integrator)
- [cpp-kit](https://github.com/ismawno/cpp-kit)
- [yaml-cpp](https://github.com/ismawno/yaml-cpp) (optional)
- [spdlog](https://github.com/gabime/spdlog) (optional)
- [glfw](https://github.com/glfw/glfw)
- [imgui](https://github.com/ocornut/imgui)
- [implot](https://github.com/epezent/implot)

## Setup

### MacOS

Run the [setup-macos.sh](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-macos.sh) script. This will automatically install the necessary software: XCode Command Line Tools, homebrew, python (3.5 of later), CMake, premake, and make. It will also generate the project build files using gmake, the only supported generator on this platform.

### Windows

Run the [setup-win-vs.bat](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-win-vs.bat) (Visual Studio) or the [setup-win-mingw.bat](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/setup-win-mingw.bat) (MinGW) script. This will automatically install the necessary software. It will also generate the project build files using the selected generator.

### Generating Build Files

After completing the setup, and if it was not already done automatically, manually execute the [generate_build_files.py](https://github.com/ismawno/poly-physx-demo/blob/dev/scripts/generate_build_files.py) script. This script will generate the project build files depending on the generator. The script is executed the same way for both operating systems, with the default generator changing depending on the platform. For Windows, if gmake is chosen as the generator, the MinGW generator will be used, and the script will automatically install the software if not found.

### Compilation

#### MacOS

From the root, simply run the `make` command, and the project will compile with the default configuration. Type `make help` for a list of configurations.

#### Windows

For Visual Studio, open the generated solution and compile the project from there. Visual Studio should handle the rest
For gmake with MinGW, from the root, simply run the `mingw32-make` command, and the project will compile with the default configuration. Type `mingw32-make help` for a list of configurations.

## License

poly-physx-demo is licensed under the MIT License. See LICENSE for more information.