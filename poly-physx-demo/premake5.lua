project "poly-physx-demo"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"
   libdirs "../vendor/SFML/build-sfml/lib"
   links {"vec-2D", "profile-tools", "ini-parser", "rk-integrator", "shapes-2D", "poly-physx", "imgui", "imgui-SFML", "implot", "poly-physx-app", "sfml-primitives", "sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}