workspace "physics-engine-2D"
    architecture "arm64"
    configurations {"debug", "release", "test", "debug-profile", "release-profile", "test-profile"}
    startproject "app"
    buildoptions "-Wall"
    linkoptions "-Wl,-rpath,./SFML/build-sfml/lib"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
defines "HAS_IMPLOT"

filter "configurations:debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "On"

   filter "configurations:test"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:debug-profile"
      defines { "DEBUG", "PERF" }
      runtime "Debug"
      symbols "On"

   filter "configurations:release-profile"
      defines { "NDEBUG", "PERF" }
      runtime "Release"
      optimize "On"

   filter "configurations:test-profile"
      defines { "DEBUG", "PERF" }
      runtime "Debug"
      symbols "On"

include "vec-2D"
include "runge-kutta"
include "shapes-2D"
include "ini-parser"
include "profile-tools"
include "engine"
include "imgui"
include "imgui-sfml"
include "implot"
include "phys-app"
include "sfml-primitives"
include "poly-physx-demo"

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