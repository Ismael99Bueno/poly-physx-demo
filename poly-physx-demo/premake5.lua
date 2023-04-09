project "poly-physx-demo"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"

   links {"vec-2D", "profile-tools", "ini-parser", "rk-integrator", "shapes-2D", "poly-physx", "imgui", "imgui-SFML", "implot", "poly-physx-app"}
   filter "system:macosx"
      libdirs "../vendor/SFML/build-sfml/lib"
      links {"sfml-primitives", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}
   filter "system:windows"
      
      links {"opengl32", "freetype", "winmm", "gdi32"}
      filter "configurations:debug*"
         libdirs "../vendor/SFML/build-sfml/lib/Debug"
         links {"sfml-graphics-s-d", "sfml-window-s-d", "sfml-system-s-d"}
         
      filter "configurations:release*"
         libdirs "../vendor/SFML/build-sfml/lib/Release"
         links {"sfml-graphics-s", "sfml-window-s", "sfml-system-s"}