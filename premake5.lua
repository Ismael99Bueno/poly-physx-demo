workspace "physics-engine-2D"
    architecture "arm64"
    configurations {"Debug", "Release", "Distribution", "Test"}
    startproject "app"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "vector"
include "runge-kutta"
include "geometry"
include "vec-ptr"
include "profiling"
include "engine"
include "app"

newaction {
    trigger = "clean",
    description = "Remove all object and binary files",
    execute = function()
        print("Removing binaries")
        os.rmdir("**/bin")
        os.rmdir("./bin")
        print("Removing objects")
        os.rmdir("**/build")
        os.rmdir("./build")
        print("Removing Makefiles")
        os.remove("**/Makefile")
        os.remove("Makefile")
        print("Done")
    end
}