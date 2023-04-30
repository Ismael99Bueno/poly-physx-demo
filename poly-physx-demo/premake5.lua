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

pchheader "pch.hpp"
pchsource "src/pch.cpp"

targetdir("bin/" .. outputdir)
objdir("build/" .. outputdir)

files {
   "src/**.cpp",
   "include/**.hpp"
}

includedirs {
   "../**/include",
   "../vendor/glm"
}

links {
   "profile-tools",
   "ini-parser",
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
libdirs "../vendor/SFML/build-gmake/lib"
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
libdirs "../vendor/SFML/build-vs/lib/Debug"
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
libdirs "../vendor/SFML/build-vs/lib/Release"
links {
   "sfml-graphics-s",
   "sfml-window-s",
   "sfml-system-s"
}

filter {
   "system:windows",
   "action:gmake*"
}
libdirs "../vendor/SFML/build-gmake/lib"
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
libdirs "../vendor/SFML/extlibs/libs-msvc/x64"

filter {
   "system:windows",
   "platforms:x86",
   "action:vs*"
}
libdirs "../vendor/SFML/extlibs/libs-msvc/x86"

filter {
   "system:windows",
   "platforms:x86_64",
   "action:gmake*"
}
libdirs "../vendor/SFML/extlibs/libs-mingw/x64"

filter {
   "system:windows",
   "platforms:x86",
   "action:gmake*"
}
libdirs "../vendor/SFML/extlibs/libs-mingw/x86"

filter "system:windows"
links {
   "opengl32",
   "freetype",
   "winmm",
   "gdi32"
}
