#ifndef PCH_HPP
#define PCH_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <imgui.h>
#include <imgui-SFML.h>
#include "implot.h"
#include <optional>
#include <utility>
#include <unordered_set>
#include <vector>
#include <variant>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/norm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>
#include <filesystem>
#include <cmath>
#include <random>
#include <string>
#include <limits>
#ifdef HAS_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif
#include <functional>
#include "ppx-app/pch.hpp"
#ifdef HAS_DEBUG_LOG_TOOLS
#include "dbg/log.hpp"
#endif

#endif