// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "Scenario002.h"

#include "ECS/System.h"

namespace {
    struct ScaleComponent {
        glm::vec3 value;
    };
    struct RotationComponent {
        glm::quat value;
    };
    struct TranslateComponent {
        glm::vec3 value;
    };
    struct TransformComponent {
        glm::mat4 value;
    };
    struct LifeCycleComponent {
        float value;
    };
    class ArchType {
    public:
        [[nodiscard]] constexpr ECS::HashSizePairs GetHashSizePairs() const noexcept { return _hashSizePairs; }

    private:
        ECS::HashSizePairs _hashSizePairs = {
            { typeid(ScaleComponent).hash_code(), sizeof(ScaleComponent) },
        };
    };
}

namespace Scenario {
    ScenarioChunkECS::ScenarioChunkECS() {
        fmt::print("Start chunk ecs scenario.\n");

        ECS::Engine ecsEngine; {
            Util::Timer timer;

            ArchType archType;
            ecsEngine.RegistryTypeInformation(archType.GetHashSizePairs());
        }
    }

    ScenarioChunkECS::~ScenarioChunkECS() {
        fmt::print("End chunk ecs scenario.\n");
        fmt::print("Press any key to end...\n");
        (void)_getch();
    }
}
