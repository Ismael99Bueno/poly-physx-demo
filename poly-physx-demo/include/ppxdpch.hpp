#ifndef PCH_HPP
#define PCH_HPP

#include "implot.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#ifdef HAS_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif
#include "ppx-app/pch.hpp"
#include <functional>
#ifdef HAS_DEBUG_LOG_TOOLS
#include "dbg/log.hpp"
#endif

#endif