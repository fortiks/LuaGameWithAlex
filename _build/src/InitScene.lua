print("[LUA] Scene is currently Initialing")

TARGET_BOUNDING_BOX = nil

BOSS_TREASURE_ENTITY = scene.CreateEntity()
LUA_FILES_FOLDER = "src\\"


HAS_GAME_BEEN_LOADED = false
HAS_REACHED_BOSS_ROOM = false
PLAYER_ALIVE = true
PLAYER_WON = false

saves = {}

molesStunned = {}

collisionInfo = {
    isColliding = false,
    distanceToCollision = 0,
    normal = {
        x = 0,
        y = 0,
        z = 0
    }
}

function getMaterialNameFromId(id)
    for materialName, materialInfo in pairs(BLOCK_INFO) do
        if materialInfo.id == id then
            return tostring(materialName)
        end
    end
    return nil
end

BLOCK_INFO = {
    UNBREAKABLE = {
        id = 0,
        name = "unbreakable",
        value = 0,
        weight = 0
    },
    SURFACE_GRASS = {
        id = 1,
        name = "surfaceGrass",
        value = 0,
        weight = 0
    },
    FINAL_LEVEL_WALL = {
        id = 2,
        name = "finalLevelWall",
        value = 0,
        weight = 0
    },
    FINAL_LEVEL_GROUND = {
        id = 3,
        name = "finalLevelGround",
        value = 0,
        weight = 0
    },
    EMPTY = {
        id = 4,
        name = "empty",
        value = 0,
        weight = 0
    },
    GRASS = {
        id = 5,
        name = "grass",
        value = 0,
        weight = 0
    },
    DIRT = {
        id = 6,
        name = "dirt",
        value = 5,
        weight = 1
    },
    STONE = {
        id = 7,
        name = "stone",
        value = 50,
        weight = 2
    },
    BRONZE = {
        id = 8,
        name = "bronze",
        value = 200,
        weight = 2
    },
    SILVER = {
        id = 9,
        name = "silver",
        value = 1000,
        weight = 3
    },
    GOLDEN = {
        id = 10,
        name = "golden",
        value = 4000,
        weight = 4
    },
    DIAMOND = {
        id = 11,
        name = "diamond",
        value = 12000,
        weight = 5
    },
    ANCIENT_RELIC = {
        id = 12,
        name = "ancientRelic",
        value = 1000000,
        weight = 1
    }
}

function chooseMaterial(probabilities)
    local randNr = math.random()
    for material, materialProbabilities in pairs(probabilities) do
        if randNr >= materialProbabilities.min and randNr <= materialProbabilities.max then
            return BLOCK_INFO[material].value, BLOCK_INFO[material].weight, BLOCK_INFO[material].id
        end
    end
end

function chooseMaterial(probabilities)
    local randNr = math.random()
    for material, materialProbabilities in pairs(probabilities) do
        if randNr >= materialProbabilities.min and randNr <= materialProbabilities.max then
            return BLOCK_INFO[material].value, BLOCK_INFO[material].weight, BLOCK_INFO[material].id
        end
    end
end

function randomizeSlice(levelNr, sliceNr)
    local currentLevel = levels[levelNr]
    local y = currentLevel.beginningDepth + sliceNr - 1
    for z = 0, HEIGHT - 1 do
        for x = 0, WIDTH - 1 do
            local blockEntityId = blockEntityIds[y * WIDTH * HEIGHT + z * HEIGHT + x + 1]
            local value, weight, chosenMaterial = chooseMaterial(currentLevel.probabilities)
            scene.SetComponent(blockEntityId, "block", value, weight, {
                x = START_X + x * BLOCK_SIZE,
                y = START_Y - y * BLOCK_SIZE,
                z = START_Z + z * BLOCK_SIZE
            }, {
                x = BLOCK_SIZE,
                y = BLOCK_SIZE,
                z = BLOCK_SIZE
            }, chosenMaterial
            )
        end
    end
end

LEVEL_DEPTH = 10

levels = {
    {
        nrOfMoles = 2,
        beginningDepth = 1,
        depth = LEVEL_DEPTH,
        probabilities = {
            DIRT = {
                min = 0,
                max = 0.8
            },
            BRONZE = {
                min = 0.8,
                max = 0.9
            },
            SILVER = {
                min = 0.9,
                max = 0.97
            },
            GOLDEN = {
                min = 0.97,
                max = 1
            }
        }
    },
    {
        nrOfMoles = 3,
        beginningDepth = 1 * LEVEL_DEPTH + 1,
        depth = 2 * LEVEL_DEPTH,
        probabilities = {
            DIRT = {
                min = 0,
                max = 0.6
            },
            STONE = {
                min = 0.6,
                max = 0.7
            },
            BRONZE = {
                min = 0.7,
                max = 0.75
            },
            SILVER = {
                min = 0.75,
                max = 0.9
            },
            GOLDEN = {
                min = 0.9,
                max = 0.99
            },
            DIAMOND = {
                min = 0.99,
                max = 1
            }
        }
    },
    {
        nrOfMoles = 3,
        beginningDepth = 2 * LEVEL_DEPTH + 1,
        depth = 3 * LEVEL_DEPTH,
        probabilities = {
            STONE = {
                min = 0.0,
                max = 0.7
            },
            BRONZE = {
                min = 0.7,
                max = 0.8
            },
            SILVER = {
                min = 0.8,
                max = 0.87
            },
            GOLDEN = {
                min = 0.87,
                max = 0.95
            },
            DIAMOND = {
                min = 0.95,
                max = 0.992
            },
            ANCIENT_RELIC = {
                min = 0.992,
                max = 1
            }
        }
    }
}

math.randomseed(math.floor(os.clock() * 1000))

blockEntityIds = {}

NUMBER_OF_LEVELS = 3
LEVEL_DEPTH = 10
MOLE_ROOM_SIDE_LENGTH = 7

WORLD_WIDTH = 250
WORLD_HEIGHT = 250
WIDTH = 15
HEIGHT = 15
BLOCK_SIZE = 3
START_X = BLOCK_SIZE / 2
START_Z = BLOCK_SIZE / 2
START_Y = -BLOCK_SIZE / 2

MOLE_SIZE = 1.5
TREASURE_SIZE = 1

CENTER_X = WIDTH / 2
CENTER_Z = HEIGHT / 2
FINAL_LEVEL_WIDTH = 2 * WIDTH
FINAL_LEVEL_HEIGHT = 2 * HEIGHT
FINAL_LEVEL_DEPTH = 5

staticBlockEntityIds = {}
surroundingBlockEntities = {
    scene.CreateEntity(), -- WIDE1
    scene.CreateEntity(), -- WIDE2
    scene.CreateEntity(), -- SMALL1
    scene.CreateEntity(), -- SMALL2
    scene.CreateEntity(), -- WIDEBELOW1
    scene.CreateEntity(), -- WIDEBELOW2
    scene.CreateEntity(), -- SMALLBELOW1
    scene.CreateEntity(), -- SMALLBELOW2
    scene.CreateEntity(), -- GROUNDBELOW
    scene.CreateEntity(), -- FINAL_LEVEL_WIDE1
    scene.CreateEntity(), -- FINAL_LEVEL_WIDE2
    scene.CreateEntity(), -- FINAL_LEVEL_SMALL1
    scene.CreateEntity() -- FINAL_LEVEL_SMALL2
}

for _, id in ipairs(surroundingBlockEntities) do
    table.insert(staticBlockEntityIds, id)
end

scene.SetComponent(surroundingBlockEntities[1], "block", BLOCK_INFO.SURFACE_GRASS.value, BLOCK_INFO.SURFACE_GRASS.weight
    ,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = -1.5 * BLOCK_SIZE,
        z = ((CENTER_Z - HEIGHT / 2) - (WORLD_HEIGHT / 2 - HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = WORLD_WIDTH * BLOCK_SIZE,
        y = BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.SURFACE_GRASS.id
)
scene.SetComponent(surroundingBlockEntities[2], "block", BLOCK_INFO.SURFACE_GRASS.value, BLOCK_INFO.SURFACE_GRASS.weight
    ,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = -1.5 * BLOCK_SIZE,
        z = ((CENTER_Z + HEIGHT / 2) + (WORLD_HEIGHT / 2 - HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = WORLD_WIDTH * BLOCK_SIZE,
        y = BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.SURFACE_GRASS.id
)

scene.SetComponent(surroundingBlockEntities[3], "block", BLOCK_INFO.SURFACE_GRASS.value, BLOCK_INFO.SURFACE_GRASS.weight
    ,
    {
        x = (CENTER_X + WIDTH / 2 + ((WORLD_WIDTH - WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = -1.5 * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - WIDTH) / 2 * BLOCK_SIZE,
        y = BLOCK_SIZE,
        z = HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.SURFACE_GRASS.id
)

scene.SetComponent(surroundingBlockEntities[4], "block", BLOCK_INFO.SURFACE_GRASS.value, BLOCK_INFO.SURFACE_GRASS.weight
    ,
    {
        x = (CENTER_X - WIDTH / 2 - ((WORLD_WIDTH - WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = -1.5 * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - WIDTH) / 2 * BLOCK_SIZE,
        y = BLOCK_SIZE,
        z = HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.SURFACE_GRASS.id
)

scene.SetComponent(surroundingBlockEntities[5], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = (-2 - (#levels / 2) * LEVEL_DEPTH) * BLOCK_SIZE,
        z = ((CENTER_Z - HEIGHT / 2) - (WORLD_HEIGHT / 2 - HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = WORLD_WIDTH * BLOCK_SIZE,
        y = (#levels * LEVEL_DEPTH) * BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[6], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = (-2 - (#levels / 2) * LEVEL_DEPTH) * BLOCK_SIZE,
        z = ((CENTER_Z + HEIGHT / 2) + (WORLD_HEIGHT / 2 - HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = WORLD_WIDTH * BLOCK_SIZE,
        y = (#levels * LEVEL_DEPTH) * BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[7], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X + WIDTH / 2 + ((WORLD_WIDTH - WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = (-2 - (#levels / 2) * LEVEL_DEPTH) * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - WIDTH) / 2 * BLOCK_SIZE,
        y = (#levels * LEVEL_DEPTH) * BLOCK_SIZE,
        z = HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[8], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X - WIDTH / 2 - ((WORLD_WIDTH - WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = (-2 - (#levels / 2) * LEVEL_DEPTH) * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - WIDTH) / 2 * BLOCK_SIZE,
        y = (#levels * LEVEL_DEPTH) * BLOCK_SIZE,
        z = HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[9], "block", BLOCK_INFO.STONE.value, BLOCK_INFO.STONE.weight, {
    x = (CENTER_X) * BLOCK_SIZE,
    y = START_Y - (2 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
    z = (CENTER_Z) * BLOCK_SIZE
}, {
    x = FINAL_LEVEL_WIDTH * BLOCK_SIZE,
    y = BLOCK_SIZE,
    z = FINAL_LEVEL_HEIGHT * BLOCK_SIZE
}, BLOCK_INFO.STONE.id
)

scene.SetComponent(surroundingBlockEntities[10], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = START_Y + (-(2 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) + FINAL_LEVEL_DEPTH / 2) * BLOCK_SIZE,
        z = ((CENTER_Z - FINAL_LEVEL_HEIGHT / 2) - (WORLD_HEIGHT / 2 - FINAL_LEVEL_HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH) * BLOCK_SIZE,
        y = (FINAL_LEVEL_DEPTH + 1) * BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[11], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X) * BLOCK_SIZE,
        y = START_Y + (-(2 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) + FINAL_LEVEL_DEPTH / 2) * BLOCK_SIZE,
        z = ((CENTER_Z + FINAL_LEVEL_HEIGHT / 2) + (WORLD_HEIGHT / 2 - FINAL_LEVEL_HEIGHT / 2) / 2) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH) * BLOCK_SIZE,
        y = (FINAL_LEVEL_DEPTH + 1) * BLOCK_SIZE,
        z = (WORLD_HEIGHT / 2 - FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[12], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X + FINAL_LEVEL_WIDTH / 2 + ((WORLD_WIDTH - FINAL_LEVEL_WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = START_Y + (-(2 + FINAL_LEVEL_DEPTH + #levels * LEVEL_DEPTH) + FINAL_LEVEL_DEPTH / 2) * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - FINAL_LEVEL_WIDTH) / 2 * BLOCK_SIZE,
        y = (FINAL_LEVEL_DEPTH + 1) * BLOCK_SIZE,
        z = FINAL_LEVEL_HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

scene.SetComponent(surroundingBlockEntities[13], "block", BLOCK_INFO.UNBREAKABLE.value, BLOCK_INFO.UNBREAKABLE.weight,
    {
        x = (CENTER_X - FINAL_LEVEL_WIDTH / 2 - ((WORLD_WIDTH - FINAL_LEVEL_WIDTH) / 2) / 2) * BLOCK_SIZE,
        y = START_Y + (-(2 + FINAL_LEVEL_DEPTH + #levels * LEVEL_DEPTH) + FINAL_LEVEL_DEPTH / 2) * BLOCK_SIZE,
        z = (CENTER_Z) * BLOCK_SIZE
    }, {
        x = (WORLD_WIDTH - FINAL_LEVEL_WIDTH) / 2 * BLOCK_SIZE,
        y = (FINAL_LEVEL_DEPTH + 1) * BLOCK_SIZE,
        z = FINAL_LEVEL_HEIGHT * BLOCK_SIZE
    }, BLOCK_INFO.UNBREAKABLE.id
)

print("[LUA] Scene Initialized")
