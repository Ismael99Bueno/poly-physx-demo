project "app"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "/opt/homebrew/Cellar/sfml/2.5.1_2/include"}
   libdirs {"/opt/homebrew/Cellar/sfml/2.5.1_2/lib", "/System/Library/Frameworks"}
   links {"vector", "profiling", "vec-ptr", "runge-kutta", "geometry", "engine", "imgui", "phys-app", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}

   filter "configurations:debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"
      removefiles "src/test.cpp"

   filter "configurations:release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "On"
      removefiles "src/test.cpp"

   filter "configurations:test"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"
      removefiles "src/main.cpp"

   filter "configurations:debug-profile"
      defines { "DEBUG", "PERF" }
      runtime "Debug"
      symbols "On"
      removefiles "src/test.cpp"

   filter "configurations:release-profile"
      defines { "NDEBUG", "PERF" }
      runtime "Release"
      optimize "On"
      removefiles "src/test.cpp"

   filter "configurations:test-profile"
      defines { "DEBUG", "PERF" }
      runtime "Debug"
      symbols "On"
      removefiles "src/main.cpp"