project "poly-physx-demo"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"
   libdirs "../vendor/SFML/build-sfml/lib"--, "/opt/homebrew/Cellar/libomp/15.0.6/lib"}
   links {"vec-2D", "profile-tools", "ini-parser", "rk-integrator", "shapes-2D", "poly-physx", "imgui", "imgui-SFML", "implot", "poly-physx-app", "sfml-primitives", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}

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