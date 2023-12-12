project "poly-physx-demo"
kind "ConsoleApp"
staticruntime "off"

defines {'PPX_DEMO_ROOT_PATH="' .. rootpath .. '"'}

language "C++"
cppdialect "C++17"
filter "system:macosx"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter"
   }

pchheader "ppx-demo/internal/pch.hpp"
pchsource "src/internal/pch.cpp"

targetdir("bin/" .. outputdir)
objdir("build/" .. outputdir)

files {
   "src/**.cpp",
   "include/**.hpp"
}

includedirs {
   "include",
   "%{wks.location}/poly-physx/include",
   "%{wks.location}/poly-physx-app/include",
   "%{wks.location}/lynx/include",
   "%{wks.location}/geometry/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/cpp-kit/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui",
   "%{wks.location}/vendor/implot",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/vendor/glfw/include",
   "%{wks.location}/vendor/spdlog/include"
}

links {
   "glfw",
   "Cocoa.framework",
   "IOKit.framework",
   "CoreFoundation.framework",
   "vulkan",
   "yaml-cpp",
   "rk-integrator",
   "geometry",
   "poly-physx",
   "cpp-kit",
   "imgui",
   "implot",
   "poly-physx-app",
   "lynx",
}

