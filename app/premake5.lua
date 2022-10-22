project "app-2D"
   kind "ConsoleApp"

   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "/opt/homebrew/Cellar/sfml/2.5.1_2/include"}
   --libdirs "/opt/homebrew/Cellar/sfml/2.5.1_2/lib"
   --links {"vector", "runge-kutta", "vec-ptr", "timer", "engine-2D", "sfml-graphics", "sfml-window", "sfml-system"}

   filter "configurations:Debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"
      removefiles "src/test.cpp"

   filter "configurations:Release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "On"
      removefiles "src/test.cpp"

   filter "configurations:Test"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"
      removefiles "src/main.cpp"