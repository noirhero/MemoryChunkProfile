// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#pragma once

namespace Math {
    namespace Vec3 {
        const glm::vec3 Zero{ 0.0f };
        const glm::vec3 One{ 1.0f };
        const glm::vec3 AxisX{ 1.0f, 0.0f, 0.0f };
        const glm::vec3 AxisY{ 0.0f, 1.0f, 0.0f };
        const glm::vec3 AxisZ{ 0.0f, 0.0f, 1.0f };
    }
    namespace Quat {
        const glm::quat Identity{ 1.0f, 0.0f, 0.0f, 0.0f };
    }
    namespace Mat4 {
        const glm::mat4 Identity{ 1.0f };
    }
}
