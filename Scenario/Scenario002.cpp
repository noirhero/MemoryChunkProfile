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
    struct LifeComponent {
        float value;
    };

    namespace ArchType {
        [[nodiscard]] constexpr ECS::Hashes GetHashes() noexcept {
            return {
                typeid(ScaleComponent).hash_code(),
                typeid(RotationComponent).hash_code(),
                typeid(TranslateComponent).hash_code(),
                typeid(TransformComponent).hash_code(),
                typeid(LifeComponent).hash_code(),
            };
        }
        [[nodiscard]] constexpr ECS::HashSizePairs GetHashSizePairs() noexcept {
            return {
                { typeid(ScaleComponent).hash_code(), static_cast<ECS::Size>(sizeof(ScaleComponent)) },
                { typeid(RotationComponent).hash_code(), static_cast<ECS::Size>(sizeof(RotationComponent)) },
                { typeid(TranslateComponent).hash_code(), static_cast<ECS::Size>(sizeof(TranslateComponent)) },
                { typeid(TransformComponent).hash_code(), static_cast<ECS::Size>(sizeof(TransformComponent)) },
                { typeid(LifeComponent).hash_code(), static_cast<ECS::Size>(sizeof(LifeComponent)) },
            };
        }
    };

    class PrintScreenSystem final : public ECS::System {
    public:
        explicit PrintScreenSystem(const Util::Timer& timer, float interval)
            : ECS::System({})
            , _timer(timer), _interval(interval) {
        }

        void Run(ECS::Engine& ecsEngine, float delta) override {
            _checkTime -= delta;
            if(0.0f < _checkTime) {
                return;
            }
            _checkTime += _interval;

            _ratioFrame = 0 == _ratioFrame ? _timer.Frame() : (_ratioFrame + _timer.Frame()) / 2;

            system("cls");

            fmt::print("Total entity count : {}\n", ecsEngine.GetNumTotalEntity());
            fmt::print("Total time         : {}\n", _timer.Total());
            fmt::print("FPS                : {}\n", _timer.Frame());
            fmt::print("Ratio FPS          : {}\n", _ratioFrame);
        }

    private:
        const Util::Timer& _timer;
        const float        _interval;
        float              _checkTime = 0.0f;
        uint32_t           _ratioFrame = 0;
    };

    class CreateEntitySystem final : public ECS::System {
    public:
        explicit CreateEntitySystem(uint32_t maxCount, float minLifeSeconds, float maxLifeSeconds)
            : ECS::System(ArchType::GetHashes())
            , _maxCount(maxCount), _minLifeSeconds(minLifeSeconds), _maxLifeSeconds(maxLifeSeconds) {
        }

        void Run(ECS::Engine& ecsEngine, float delta) override {
            CreateEntities(ecsEngine);
        }

    protected:
        void CreateEntities(ECS::Engine& ecsEngine) const {
            for (auto i = static_cast<decltype(_maxCount)>(ecsEngine.GetNumTotalEntity()); i < _maxCount; ++i) {
                const auto& [scale, rotation, translation, transform, lifeCycle] =
                    Chunk::Accept<ScaleComponent, RotationComponent, TranslateComponent, TransformComponent, LifeComponent>(ecsEngine.CreateEntity(_hashes)->Get(_hashes));

                scale->value = Math::Vec3::One;
                rotation->value = Math::Quat::Identity;
                translation->value = Math::Vec3::Zero;
                transform->value = Math::Mat4::Identity;
                lifeCycle->value = Util::Random::Distribution(_minLifeSeconds, _maxLifeSeconds);
            }
        }

    private:
        const uint32_t _maxCount;
        const float    _minLifeSeconds;
        const float    _maxLifeSeconds;
    };

    class DestroyEntitySystem final : public ECS::System {
    public:
        explicit DestroyEntitySystem(ECS::Engine& ecsEngine)
            : ECS::System({ typeid(LifeComponent).hash_code() })
            , _ecsEngine(ecsEngine) {
        }

    protected:
        void ForEach(const ECS::Collector& collector, float delta) override {
            auto* lifeCycles = Accept<LifeComponent>(collector);

            for(std::remove_const_t<decltype(collector.count)> i = 0; i < collector.count; ++i) {
                lifeCycles[i].value -= delta;
                if (0.0f >= lifeCycles[i].value) {
                    _ecsEngine.DestroyEntity(collector.handler, i);
                }
            }
        }

    private:
        ECS::Engine& _ecsEngine;
    };

    class RotationSystem final : public ECS::System {
    public:
        RotationSystem() : ECS::System({ typeid(RotationComponent).hash_code() }) {
        }

        void ForEach(const ECS::Collector& collector, float delta) override {
            auto* rotations = Accept<RotationComponent>(collector);
            for(std::remove_const_t<decltype(collector.count)> i = 0; i < collector.count; ++i) {
                rotations[i].value = glm::rotate(rotations[i].value, delta, Math::Vec3::AxisY);
            }
        }
    };

    class TransformSystem final : public ECS::System {
    public:
        TransformSystem() : ECS::System({
            typeid(ScaleComponent).hash_code(),
            typeid(RotationComponent).hash_code(),
            typeid(TranslateComponent).hash_code(),
            typeid(TransformComponent).hash_code(),
        }) {
        }

        void ForEach(const ECS::Collector& collector, float delta) override {
            const auto& [scales, rotations, translations, transforms] = 
                Accept<ScaleComponent, RotationComponent, TranslateComponent, TransformComponent>(collector);

            for(std::remove_const_t<decltype(collector.count)> i = 0; i < collector.count; ++i) {
                const auto scaleTm = glm::scale(Math::Mat4::Identity, scales[i].value);
                const auto rotationTm = glm::toMat4(rotations[i].value);
                const auto posTm = glm::translate(Math::Mat4::Identity, translations[i].value);
                transforms[i].value = posTm * rotationTm * scaleTm;
            }
        }
    };
}

namespace Scenario {
    ScenarioChunkECS::ScenarioChunkECS() {
        fmt::print("Start chunk ecs scenario.\n");

        ECS::Engine ecsEngine;
        ecsEngine.RegistryTypeInformation(ArchType::GetHashSizePairs()); {
            Util::Timer timer;

            PrintScreenSystem printScreenSystem(timer, 1.0f);
            CreateEntitySystem createSystem(NumEntities, 1.0f, 10.0f);
            DestroyEntitySystem destroySystem(ecsEngine);
            RotationSystem rotationSystem;
            TransformSystem transformSystem;

            while(60.0f > timer.Total()) {
                timer.Update();

                printScreenSystem.Run(ecsEngine, timer.Delta());
                createSystem.Run(ecsEngine, timer.Delta());
                destroySystem.Run(ecsEngine, timer.Delta());
                rotationSystem.Run(ecsEngine, timer.Delta());
                transformSystem.Run(ecsEngine, timer.Delta());
            }
        }
    }

    ScenarioChunkECS::~ScenarioChunkECS() {
        fmt::print("End chunk ecs scenario.\n");
        fmt::print("Press any key to end...\n");
        (void)_getch();
    }
}
