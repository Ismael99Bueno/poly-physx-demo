project "poly-physx-demo"
kind "ConsoleApp"
staticruntime "off"

defines {'ROOT_PATH="' .. rootpath .. '"'}

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

   rpath = "-Wl,-rpath,".. rootpath .."vendor/vulkan-sdk/macOS/lib"
   linkoptions {rpath}

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
   "%{wks.location}/shapes-2D/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/cpp-kit/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui",
   "%{wks.location}/vendor/implot/include",
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
   "shapes-2D",
   "poly-physx",
   "cpp-kit",
   "imgui",
   "implot",
   "poly-physx-app",
   "lynx",
}

