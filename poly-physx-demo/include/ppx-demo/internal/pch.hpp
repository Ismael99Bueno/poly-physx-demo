#ifndef PPX_DEMO_PCH_HPP
#define PPX_DEMO_PCH_HPP

#include "ppx-app/internal/pch.hpp"
#ifdef LYNX_ENABLE_IMGUI
#include <imgui.h>
#ifdef LYNX_ENABLE_IMPLOT
#include <implot.h>
#endif
#endif
#include <filesystem>

#endif