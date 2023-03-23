project "phys-demo"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"
   --libdirs {"/opt/homebrew/Cellar/sfml/2.5.1_2/lib", "/opt/homebrew/Cellar/libomp/15.0.6/lib"}
   links {"SFML", "vec2", "profiling", "ini", "runge-kutta", "geometry", "engine", "imgui", "imgui-SFML", "implot", "phys-app", "sfml-primitives"}--, "omp", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}

   filter "configurations:debug"
      removefiles "src/test.cpp"

   filter "configurations:release"
      removefiles "src/test.cpp"

   filter "configurations:test"
      removefiles "src/main.cpp"

   filter "configurations:debug-profile"
      removefiles "src/test.cpp"

   filter "configurations:release-profile"
      removefiles "src/test.cpp"

   filter "configurations:test-profile"
      removefiles "src/main.cpp"