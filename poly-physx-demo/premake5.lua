project "poly-physx-demo"
kind "ConsoleApp"
staticruntime "off"

defines {'PPX_DEMO_ROOT_PATH="' .. rootpath .. '"'}

language "C++"
cppdialect "c++20"
filter "system:macosx"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter",
      "-Wno-sign-conversion"
   }
filter{}

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
   "poly-physx-app",
   "lynx",
   "poly-physx",
   "geometry",
   "rk-integrator",
   "cpp-kit",
   "implot",
   "imgui",
   "glfw",
   "yaml-cpp",
}
linkoptions "-rpath /usr/local/lib"

VULKAN_SDK = os.getenv("VULKAN_SDK")
filter "system:windows"
   includedirs "%{VULKAN_SDK}/Include"
   libdirs "%{VULKAN_SDK}/Lib"
   links {"vulkan-1", "gdi32"}

filter "system:macosx"
   links {
      "vulkan",
      "Cocoa.framework",
      "IOKit.framework",
      "CoreFoundation.framework"
   }


