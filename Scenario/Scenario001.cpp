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
    using Entities = std::unordered_set<Entity*>;

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
            Print();
            fmt::print("Ratio FPS          : {}\n", _ratioFrame);
        }

        void ForEach(float delta, Entity& entity) override {}
        void Run(float delta) override {
            _checkTime -= delta;
            if (0.0f >= _checkTime) {
                _checkTime = _interval;
                Print();
            }
        }

    private:
        void               Print() {
            system("cls");

            fmt::print("Total entity count : {}\n", _entities.size());
            fmt::print("Total time         : {}\n", _timer.Total());
            fmt::print("FPS                : {}\n", _timer.Frame());

            _ratioFrame = 0 == _ratioFrame ? _timer.Frame() : (_ratioFrame + _timer.Frame()) / 2;
        }

        const Util::Timer& _timer;
        const float        _interval;
        float              _checkTime = 0.0f;
        uint32_t           _ratioFrame = 0;
    };

    class CreateEntitySystem final : public System {
    public:
        explicit CreateEntitySystem(Entities& entities, size_t maxCount, float minLifeSeconds, float maxLifeSeconds)
            : System(entities)
            , _maxCount(maxCount), _minLifeSeconds(minLifeSeconds), _maxLifeSeconds(maxLifeSeconds) {
        }

        void ForEach(float, Entity&) override {}
        void Run(float delta) override {
            CreateEntities();
        }

    private:
        void CreateEntities() const {
            for(auto i = static_cast<decltype(_maxCount)>(_entities.size()); i < _maxCount; ++i) {
                auto* entity = new Entity;
                entity->components.try_emplace(typeid(ScaleComponent).hash_code(), new ScaleComponent);
                entity->components.try_emplace(typeid(RotationComponent).hash_code(), new RotationComponent);
                entity->components.try_emplace(typeid(TranslateComponent).hash_code(), new TranslateComponent);
                entity->components.try_emplace(typeid(TransformComponent).hash_code(), new TransformComponent);
                entity->components.try_emplace(typeid(LifeCycleComponent).hash_code(), new LifeCycleComponent{ Util::Random::Distribution(1.0f, 10.0f) });
                _entities.emplace(entity);
            }
        }

        const size_t _maxCount;
        const float  _minLifeSeconds;
        const float  _maxLifeSeconds;
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
                for(auto* entity : _destroyEntities) {
                    _entities.erase(entity);
                    delete entity;
                }
                _destroyEntities.clear();
            }
        }

    private:
        std::vector<Entity*> _destroyEntities;
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
            CreateEntitySystem createSystem(entities, 100000, 1.0f, 10.0f);
            DestroyEntitySystem destroySystem(entities);
            RotationSystem rotateSystem(entities);
            TransformSystem transformSystem(entities);

            while (60.0f >= timer.Total()) {
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
