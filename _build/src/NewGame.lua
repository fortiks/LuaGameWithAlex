IS_IN_EDIT_MODE = false

CHARACTER_ENTITY_ID = scene.CreateEntity()

scene.SetComponent(CHARACTER_ENTITY_ID, "behaviour", LUA_FILES_FOLDER .. "Character.lua")

for z = 0, HEIGHT - 1 do
    for x = 0, WIDTH - 1 do
        local blockEntityId = scene.CreateEntity()
        table.insert(blockEntityIds, blockEntityId)
        scene.SetComponent(blockEntityId, "block", BLOCK_INFO.GRASS.value, BLOCK_INFO.GRASS.weight, {
            x = START_X + x * BLOCK_SIZE,
            y = START_Y,
            z = START_Z + z * BLOCK_SIZE
        }, {
            x = BLOCK_SIZE,
            y = BLOCK_SIZE,
            z = BLOCK_SIZE
        }, BLOCK_INFO.GRASS.id
        )
    end
end

for levelNr, level in ipairs(levels) do
    for y = level.beginningDepth, level.depth do
        for _ = 0, HEIGHT - 1 do
            for _ = 0, WIDTH - 1 do
                local blockEntityId = scene.CreateEntity()
                table.insert(blockEntityIds, blockEntityId)
            end
        end
        randomizeSlice(levelNr, (y - 1) % (LEVEL_DEPTH) + 1)
    end
end

for z = 0, WIDTH - 1 do
    for x = 0, HEIGHT - 1 do
        scene.SetComponent(blockEntityIds[z * WIDTH + x + 1], "block", 0, 0, {
            x = START_X + x * BLOCK_SIZE,
            y = START_Y - BLOCK_SIZE,
            z = START_Z + z * BLOCK_SIZE
        }, {
            x = BLOCK_SIZE,
            y = BLOCK_SIZE,
            z = BLOCK_SIZE
        }, BLOCK_INFO.EMPTY.id
        )
    end
end
for z = 0, WIDTH - 1 do
    for x = 0, HEIGHT - 1 do
        scene.SetComponent(blockEntityIds[HEIGHT * WIDTH + z * WIDTH + x + 1], "block", 0, 0, {
            x = START_X + x * BLOCK_SIZE,
            y = START_Y - BLOCK_SIZE,
            z = START_Z + z * BLOCK_SIZE
        }, {
            x = BLOCK_SIZE,
            y = BLOCK_SIZE,
            z = BLOCK_SIZE
        }, BLOCK_INFO.GRASS.id
        )
    end
end

for levelCount, level in ipairs(levels) do
    local availableDepths = {}
    for depth = 1, LEVEL_DEPTH do
        availableDepths[depth] = true
    end

    for _ = 1, level.nrOfMoles do
        local depthFound = false
        local randomDepth
        while not depthFound do
            randomDepth = math.random(level.beginningDepth + 1, level.depth - 1)
            if availableDepths[randomDepth - (levelCount - 1) * LEVEL_DEPTH] and
                availableDepths[randomDepth - (levelCount - 1) * LEVEL_DEPTH - 1] and
                availableDepths[randomDepth - (levelCount - 1) * LEVEL_DEPTH + 1] then

                availableDepths[randomDepth - (levelCount - 1) * LEVEL_DEPTH] = false
                depthFound = true
            end
        end

        local reservedBlocks = (MOLE_ROOM_SIDE_LENGTH - 1) / 2
        local treasurePos = {
            x = math.random(reservedBlocks, WIDTH - reservedBlocks - 1),
            y = randomDepth,
            z = math.random(reservedBlocks, HEIGHT - reservedBlocks - 1)
        }

        local treasureEntityId = scene.CreateEntity()
        scene.SetComponent(treasureEntityId, "treasure", levelCount * levelCount * 8000, levelCount * levelCount * 100,
            levelCount * levelCount * 100, {
            position = {
                x = START_X + treasurePos.x * BLOCK_SIZE,
                y = START_Y - treasurePos.y * BLOCK_SIZE - BLOCK_SIZE / 2 + TREASURE_SIZE / 2,
                z = START_Z + treasurePos.z * BLOCK_SIZE
            },
            rotation = {
                x = 0,
                y = 0,
                z = 0
            },
            scale = {
                x = TREASURE_SIZE,
                y = TREASURE_SIZE,
                z = TREASURE_SIZE
            }
        })

        scene.SetComponent(treasureEntityId, "behaviour", LUA_FILES_FOLDER .. "mole.lua")
        local roomStartX = treasurePos.x - reservedBlocks
        local roomStartZ = treasurePos.z - reservedBlocks
        for x = roomStartX, roomStartX + MOLE_ROOM_SIDE_LENGTH - 1 do
            for z = roomStartZ, roomStartZ + MOLE_ROOM_SIDE_LENGTH - 1 do
                local id = blockEntityIds[treasurePos.y * WIDTH * HEIGHT + z * WIDTH + x + 1]
                local currentBlock = scene.GetComponent(id, "block")
                currentBlock.material = BLOCK_INFO.EMPTY.id
                scene.SetComponent(id, "block", currentBlock.gold, currentBlock.weight, currentBlock.position,
                    currentBlock.size, currentBlock.material)
            end
        end
    end
end

scene.SetComponent(scene.CreateEntity(), "behaviour", LUA_FILES_FOLDER .. "BossRoom.lua")

print("[LUA] Scene initialized")
