#pragma once

class Scene;
struct GameState;

class EnemySystem
{
public:
    static void Update(
        Scene& scene,
        const GameState& gameState,
        float deltaTime
    );
};