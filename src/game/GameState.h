#pragma once

#include "engine/scene/Entity.h"

#include <vector>

struct GameState
{
    EntityId playerId =
        0;

    std::vector<EntityId> enemyIds;

    void Clear()
    {
        playerId =
            0;

        enemyIds.clear();
    }
};