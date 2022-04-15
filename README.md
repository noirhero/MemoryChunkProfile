# MemoryChunkProfile

Memory chunk ECS based on C++.  
The overall concept refers to unity dots.

```cpp
#include <ECS/System.h>

struct Translation {
    float x, y, z;
};

struct Rotation {
    float x, y, z, w;
};

class MoveSystem final : public ECS::System {
public:
    MoveSystem() : System({ typeid(Translation).hash_code() }) {
    }

protected:
    void ForEach(ECS::Engine&, const ECS::Collector& collector, float delta) override {
        auto* positions = Accept<Translation>(collector);

        for(std::remove_const_t<decltype(collector.count)> i = 0; i < collector.count; ++i) {
            // Todo : positions[i] access.
        }
    }
};

class RotationSystem final : public ECS::System {
public:
    RotationSystem() : System({ typeid(Rotation).hash_code() }) {
    }

protected:
    void ForEach(ECS::Engine&, const ECS::Collector& collector, float delta) override {
        auto* rotations = Accept<Rotation>(collector);

        for(std::remove_const_t<decltype(collector.count)> i = 0; i < collector.count; ++i) {
            // Todo : rotations[i] access.
        }
    }
};

int main() {
    ECS::Engine engine;

    engine.RegistryTypeInformation({
        { typeid(Translation).hash_code(), static_cast<ECS::Size>(sizeof Translation) },
        { typeid(Rotation).hash_code(), static_cast<ECS::Size>(sizeof Rotation) },
    });

    engine.CreateEntity({
        typeid(Translation).hash_code(),
        typeid(Rotation).hash_code(),
    });

    MoveSystem moveSystem;
    moveSystem.Run(engine, 0.0f/*delta*/);
    
    RotationSystem rotationSystem;
    rotationSystem.Run(engine, 0.0f/*delta*/);
}
```
