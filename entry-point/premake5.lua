project "app"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "/opt/homebrew/Cellar/sfml/2.5.1_2/include", "/opt/homebrew/Cellar/tgui/0.9.5/include"}
   libdirs {"/opt/homebrew/Cellar/sfml/2.5.1_2/lib", "/opt/homebrew/Cellar/tgui/0.9.5/lib"}
   links {"vector", "profiling", "vec-ptr", "runge-kutta", "geometry", "engine", "phys-env", "sfml-graphics", "sfml-window", "sfml-system", "tgui"}

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