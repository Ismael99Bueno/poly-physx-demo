workspace "poly-physx-demo"
    --architecture "arm64"
    configurations {"release", "debug", "release-profile", "debug-profile"}
    buildoptions "-Wall"
    linkoptions "-Wl,-rpath,./vendor/SFML/build-sfml/lib"

    filter "system:macosx"
      platforms {"arm64", "x86_64"}
      
      filter "platforms:arm64"
         architecture "ARM64"
      filter "platforms:x86_64"
         architecture "x86_64"
      filter {}

   filter "system:windows"
      platforms {"x86_64", "x86"}

      filter "platforms:x86_64"
         architecture "x86_64"
      filter "platforms:x86"
         architecture "x86"
      filter {}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
defines "HAS_IMPLOT"

filter {}

   filter "configurations:debug*"
      defines "DEBUG"
      runtime "Debug"
      symbols "On"

   filter "configurations:release*"
      defines "NDEBUG"
      runtime "Release"
      optimize "On"

   filter "configurations:*profile"
      defines "PERF"

include "vec-2D"
include "rk-integrator"
include "shapes-2D"
include "ini-parser"
include "profile-tools"
include "poly-physx"
include "vendor/imgui"
include "vendor/imgui-sfml"
include "vendor/implot"
include "poly-physx-app"
include "sfml-primitives"
include "poly-physx-demo"

newaction {
    trigger = "clean",
    description = "Remove all object and binary files",
    execute = function()
        print("Removing binaries...")
        os.rmdir("**/bin")
        print("Removing objects...")
        os.rmdir("**/build")
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