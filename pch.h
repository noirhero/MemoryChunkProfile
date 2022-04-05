// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

#include <conio.h>

#include <map>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>
#include <thread>
#include <deque>
#include <ranges>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#define GLM_FORCE_MESSAGES
#define GLM_FORCE_INLINE
#define GLM_FORCE_EXPLICIT_CTOR
#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <gsl/gsl>

#include "Mathmatics.h"
#include "Util.h"

constexpr uint32_t NumEntities = 50000;
