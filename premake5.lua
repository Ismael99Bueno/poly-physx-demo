workspace "physics-engine-2D"
    architecture "arm64"
    configurations {"debug", "release", "test", "debug-profile", "release-profile", "test-profile"}
    startproject "app"
    buildoptions "-Wall"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "vec2"
include "runge-kutta"
include "geometry"
include "vec-ptr"
include "ini"
include "profiling"
include "engine"
include "imgui"
include "imgui-SFML"
include "implot"
include "phys-app"
include "sfml-primitives"
include "phys-demo"

newaction {
    trigger = "clean",
    description = "Remove all object and binary files",
    execute = function()
        print("Removing binaries...")
        os.rmdir("**/bin")
        os.rmdir("./bin")
        print("Removing objects...")
        os.rmdir("**/build")
        os.rmdir("./build")
        print("Removing Makefiles...")
        os.remove("**/Makefile")
        os.remove("Makefile")
        print("Removing project files...")
        os.rmdir(".vs/")
        os.rmdir("**.xcworkspace/")
        os.rmdir("**.xcodeproj/")
        os.remove("**.xcodeproj")
        print("Done")
    end
}