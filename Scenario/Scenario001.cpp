// Copyright 2011-2021 GameParadiso, Inc. All Rights Reserved.

#include <pch.h>
#include "Scenario001.h"

namespace {
    struct Component {
    };
    struct ScaleComponent : public Component {
        glm::vec3 value = Math::Vec3::One;
    };
    struct RotationComponent : public Component {
        glm::quat value = Math::Quat::Identity;
    };
    struct TranslateComponent : public Component {
        glm::vec3 value = Math::Vec3::Zero;
    };
    struct TransformComponent : public Component {
        glm::mat4 value = Math::Mat4::Identity;
    };
    struct LifeCycleComponent : public Component {
        explicit LifeCycleComponent(float initValue) : value(initValue) {}
        float value = 0.0f;
    };
    struct Entity {
        ~Entity() {
            for (const auto* component : std::views::values(components)) {
                delete component;
            }
        }

        std::map<uint64_t, Component*> components;
    };
    using Entities = std::vector<Entity*>;

    class System {
    public:
        explicit System(Entities& entities) : _entities(entities) {
        }

        virtual void ForEach(float delta, Entity& entity) = 0;

        virtual void Run(float delta) {
            for (auto* entity : _entities) {
                if (std::ranges::any_of(_hashes, [entity](const auto eachHash)->bool {
                    return entity->components.find(eachHash) == entity->components.end();
                })) {
                    continue;
                }

                ForEach(delta, *entity);
            }
        }

    protected:
        template<typename T>
        T* Accept(Entity& entity) const {
            return reinterpret_cast<T*>(entity.components[typeid(T).hash_code()]);
        }

        Entities& _entities;
        std::vector<uint64_t> _hashes;
    };

    class PrintScreenSystem final : public System {
    public:
        PrintScreenSystem(Entities& entities, const Util::Timer& timer, float intervalTime) : System(entities), _timer(timer), _interval(intervalTime) {
        }
        ~PrintScreenSystem() {
            fmt::print("Ratio FPS          : {}\n", _ratioFrame);
        }

        void ForEach(float delta, Entity& entity) override {}
        void Run(float delta) override {
            _checkTime -= delta;
            if (0.0f >= _checkTime) {
                _checkTime = _interval;

                system("cls");

                fmt::print("Total entity count : {}\n", _entities.size());
                fmt::print("Total time         : {}\n", _timer.Total());
                fmt::print("FPS                : {}\n", _timer.Frame());

                _ratioFrame = 0 == _ratioFrame ? _timer.Frame() : (_ratioFrame + _timer.Frame()) / 2;
            }
        }

    private:
        const Util::Timer& _timer;
        const float        _interval;
        float              _checkTime = 0.0f;
        uint32_t           _ratioFrame = 0;
    };

    class CreateEntitySystem final : public System {
    public:
        explicit CreateEntitySystem(Entities& entities, float intervalTime, size_t maxCount)
            : System(entities)
            , _interval(intervalTime), _maxCount(maxCount) {

            for (uint32_t i = 0; i < 1000; ++i) {
                _entities.emplace_back(new Entity);
                auto* entity = _entities.back();
                entity->components.try_emplace(typeid(ScaleComponent).hash_code(), new ScaleComponent);
                entity->components.try_emplace(typeid(RotationComponent).hash_code(), new RotationComponent);
                entity->components.try_emplace(typeid(TranslateComponent).hash_code(), new TranslateComponent);
                entity->components.try_emplace(typeid(TransformComponent).hash_code(), new TransformComponent);
                entity->components.try_emplace(typeid(LifeCycleComponent).hash_code(), new LifeCycleComponent{ static_cast<float>(Util::Random::Get(1, 10)) });
            }
        }

        void ForEach(float, Entity&) override {}
        void Run(float delta) override {
            if (_entities.size() >= _maxCount) {
                return;
            }

            _checkTime -= delta;
            if (0.0f >= _checkTime) {
                _checkTime += _interval;

                _entities.emplace_back(new Entity);
                auto* entity = _entities.back();
                entity->components.try_emplace(typeid(ScaleComponent).hash_code(), new ScaleComponent);
                entity->components.try_emplace(typeid(RotationComponent).hash_code(), new RotationComponent);
                entity->components.try_emplace(typeid(TranslateComponent).hash_code(), new TranslateComponent);
                entity->components.try_emplace(typeid(TransformComponent).hash_code(), new TransformComponent);
                entity->components.try_emplace(typeid(LifeCycleComponent).hash_code(), new LifeCycleComponent{ static_cast<float>(Util::Random::Get(1, 10)) });
            }
        }

    private:
        const float  _interval;
        const size_t _maxCount;
        float        _checkTime = 0.0f;
    };

    class DestroyEntitySystem final : public System {
    public:
        DestroyEntitySystem(Entities& entities) : System(entities) {
            _hashes.emplace_back(typeid(LifeCycleComponent).hash_code());
        }

        void ForEach(float delta, Entity& entity) override {
            auto* lifeCycle = Accept<LifeCycleComponent>(entity);
            lifeCycle->value -= delta;
            if (0.0f >= lifeCycle->value) {
                _destroyEntities.emplace_back(&entity);
            }
        }
        void Run(float delta) override {
            System::Run(delta);

            if (false == _destroyEntities.empty()) {
                const auto removeRange = std::ranges::remove_if(_entities, [this](const auto* eachEntity)->bool {
                    return std::ranges::any_of(_destroyEntities, [eachEntity](const auto* destroyEntity)->bool {
                        return destroyEntity == eachEntity;
                    });
                });
                for (const auto* entity : _destroyEntities) {
                    delete entity;
                }
                _entities.erase(removeRange.begin(), removeRange.end());
                _destroyEntities.clear();
            }
        }

    private:
        Entities _destroyEntities;
    };

    class RotationSystem final : public System {
    public:
        RotationSystem(Entities& entities) : System(entities) {
            _hashes.emplace_back(typeid(RotationComponent).hash_code());
        }

        void ForEach(float delta, Entity& entity) override {
            auto* rotation = Accept<RotationComponent>(entity);
            rotation->value = glm::rotate(rotation->value, delta, Math::Vec3::AxisY);
        }
    };

    class TransformSystem final : public System {
    public:
        TransformSystem(Entities& entities) : System(entities) {
            _hashes.emplace_back(typeid(ScaleComponent).hash_code());
            _hashes.emplace_back(typeid(RotationComponent).hash_code());
            _hashes.emplace_back(typeid(TranslateComponent).hash_code());
            _hashes.emplace_back(typeid(TransformComponent).hash_code());
        }

        void ForEach(float delta, Entity& entity) override {
            const auto scaleTm = glm::scale(Math::Mat4::Identity, Accept<ScaleComponent>(entity)->value);
            const auto rotationTm = glm::toMat4(Accept<RotationComponent>(entity)->value);
            const auto posTm = glm::translate(Math::Mat4::Identity, Accept<TranslateComponent>(entity)->value);
            Accept<TransformComponent>(entity)->value = posTm * rotationTm * scaleTm;
        }
    };
}

namespace Scenario {
    ScenarioNoChunkECS::ScenarioNoChunkECS() {
        fmt::print("Start no chunk ecs scenario.\n");

        Entities entities; {
            Util::Timer timer;

            PrintScreenSystem printScreenSystem(entities, timer, 1.0f);
            CreateEntitySystem createSystem(entities, 0.1f, 10000);
            DestroyEntitySystem destroySystem(entities);
            RotationSystem rotateSystem(entities);
            TransformSystem transformSystem(entities);

            while (5.0f >= timer.Total()) {
                //std::this_thread::sleep_for(std::chrono::milliseconds(1));
                timer.Update();

                printScreenSystem.Run(timer.Delta());
                createSystem.Run(timer.Delta());
                destroySystem.Run(timer.Delta());
                rotateSystem.Run(timer.Delta());
                transformSystem.Run(timer.Delta());
            }
        }
        for (const auto* entity : entities) {
            delete entity;
        }
    }

    ScenarioNoChunkECS::~ScenarioNoChunkECS() {
        fmt::print("End no chunk ecs scenario.\n");
        fmt::print("Press any key to end...\n");
        (void)_getch();
    }
}
