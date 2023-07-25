require "export-compile-commands"

workspace "poly-physx-demo"
configurations {
   "release",
   "debug",
   "release-profile",
   "debug-profile"
}
startproject "poly-physx-demo"

defines {
   "YAML_CPP_GLM_COMPAT",
   "YAML_CPP_STATIC_DEFINE",
   "KIT_USE_SPDLOG",
   "KIT_USE_CUSTOM_ALLOC",
   "KIT_USE_YAML_CPP",
   "LYNX_ENABLE_IMGUI"
}

function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)")
end

rootpath = script_path()

filter "system:macosx"
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

filter "configurations:debug*"
   defines {"DEBUG", "KIT_LOG"}
   runtime "Debug"
   symbols "On"

filter "configurations:release*"
   defines "NDEBUG"
   runtime "Release"
   optimize "On"

filter "configurations:*profile"
   defines "KIT_PROFILE"
filter {}

include "rk-integrator"
include "shapes-2D"
include "poly-physx"
include "lynx"
include "poly-physx-app"
include "poly-physx-demo"
include "cpp-kit"
include "vendor/yaml-cpp"
include "vendor/imgui"
include "vendor/implot"
include "vendor/glfw"
