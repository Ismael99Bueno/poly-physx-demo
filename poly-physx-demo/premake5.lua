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

filter "system:windows"
   defines "SFML_STATIC"
filter {}

pchheader "ppxdpch.hpp"
pchsource "src/pch.cpp"

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
   "%{wks.location}/shapes-2D/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/debug-tools/include",
   "%{wks.location}/profile-tools/include",
   "%{wks.location}/container-view/include",
   "%{wks.location}/sfml-primitives/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui/include",
   "%{wks.location}/vendor/imgui-sfml/include",
   "%{wks.location}/vendor/implot/include",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/vendor/SFML/include"
}

links {
   "profile-tools",
   "yaml-cpp",
   "rk-integrator",
   "shapes-2D",
   "poly-physx",
   "imgui",
   "imgui-SFML",
   "implot",
   "poly-physx-app",
   "sfml-primitives"
}

filter "system:macosx"
   libdirs "%{wks.location}/vendor/SFML/build-gmake/lib"
   links {
      "sfml-graphics",
      "sfml-window",
      "sfml-system",
      "OpenGL.framework"
   }

filter {
   "system:windows",
   "configurations:debug*",
   "action:vs*"
}
   libdirs "%{wks.location}/vendor/SFML/build-vs/lib/Debug"
   links {
      "sfml-graphics-s-d",
      "sfml-window-s-d",
      "sfml-system-s-d"
   }

filter {
   "system:windows",
   "configurations:release*",
   "action:vs*"
}
   libdirs "%{wks.location}/vendor/SFML/build-vs/lib/Release"
   links {
      "sfml-graphics-s",
      "sfml-window-s",
      "sfml-system-s"
   }

filter {
   "system:windows",
   "action:gmake*"
}
   libdirs "%{wks.location}/vendor/SFML/build-gmake/lib"
   links {
      "sfml-graphics-s",
      "sfml-window-s",
      "sfml-system-s"
   }

filter {
   "system:windows",
   "platforms:x86_64",
   "action:vs*"
}
   libdirs "%{wks.location}/vendor/SFML/extlibs/libs-msvc/x64"

filter {
   "system:windows",
   "platforms:x86",
   "action:vs*"
}
   libdirs "%{wks.location}/vendor/SFML/extlibs/libs-msvc/x86"

filter {
   "system:windows",
   "platforms:x86_64",
   "action:gmake*"
}
   libdirs "%{wks.location}/vendor/SFML/extlibs/libs-mingw/x64"

filter {
   "system:windows",
   "platforms:x86",
   "action:gmake*"
}
   libdirs "%{wks.location}/vendor/SFML/extlibs/libs-mingw/x86"

filter "system:windows"
links {
   "opengl32",
   "freetype",
   "winmm",
   "gdi32"
}
