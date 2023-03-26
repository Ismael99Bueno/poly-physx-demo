project "poly-physx-demo"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"
   libdirs "../SFML/build-sfml/lib"--, "/opt/homebrew/Cellar/libomp/15.0.6/lib"}
   links {"vec-2D", "profile-tools", "ini-parser", "runge-kutta", "shapes-2D", "engine", "imgui", "imgui-SFML", "implot", "phys-app", "sfml-primitives", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}

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