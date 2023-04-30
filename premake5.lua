workspace "poly-physx-demo"
configurations {
   "release",
   "debug",
   "release-profile",
   "debug-profile"
}
startproject "poly-physx-demo"

function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)")
end

rootpath = script_path()

filter "system:macosx"
rpath = "-Wl,-rpath," .. rootpath .. "vendor/SFML/build-gmake/lib"
linkoptions {rpath}

platforms {
   "arm64",
   "x86_64"
}

filter "platforms:arm64"
architecture "ARM64"
filter "platforms:x86_64"
architecture "x86_64"
filter {}

filter "system:windows"
platforms {
   "x86_64",
   "x86"
}

filter "platforms:x86_64"
architecture "x86_64"
filter "platforms:x86"
architecture "x86"
filter {}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-" .. _ACTION
defines "HAS_IMPLOT"

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
