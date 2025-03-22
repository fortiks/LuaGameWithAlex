local bossRoom = {}

function bossRoom:OnCreate()
    math.randomseed(math.floor(os.clock() * 1000))
    self._treasureEntities = {
        scene.CreateEntity(),
        scene.CreateEntity(),
        scene.CreateEntity(),
        scene.CreateEntity()
    }
    self._treasureDurability = 5000
    self._treasuresRemaining = #self._treasureEntities
    self._moleSpawningCooldown = 40
    self._maxMoles = 4
    self._moleHealth = 2000 + (self._maxMoles - self._treasuresRemaining) * 500
    self._moleDamage = 30 + (self._maxMoles - self._treasuresRemaining) * 10
    self._bossMoleSize3 = {
        x = 4 * MOLE_SIZE,
        y = 4 * MOLE_SIZE,
        z = 4 * MOLE_SIZE
    }
    self._moles = {
        {
            id = nil,
            alive = false,
            timeKilled = os.clock()
        },
        {
            id = nil,
            alive = false,
            timeKilled = os.clock()
        },
        {
            id = nil,
            alive = false,
            timeKilled = os.clock()
        },
        {
            id = nil,
            alive = false,
            timeKilled = os.clock()
        }
    }
    self._moleSpawnPoints = {
        {
            x = 3 * TREASURE_SIZE / 2 + (3 + CENTER_X - FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
            y = START_Y + 4 * MOLE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
            z = 3 * TREASURE_SIZE / 2 + (3 + CENTER_Z - FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
        },
        {
            x = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_X + FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
            y = START_Y + 4 * MOLE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
            z = 3 * TREASURE_SIZE / 2 + (3 + CENTER_Z - FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
        },
        {
            x = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_X + FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
            y = START_Y + 4 * MOLE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
            z = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_Z + FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
        },
        {
            x = 3 * TREASURE_SIZE / 2 + (3 + CENTER_X - FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
            y = START_Y + 4 * MOLE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
            z = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_Z + FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
        }
    }

    self._update = coroutine.create(function()
        scene.SetComponent(self._treasureEntities[1], "treasure", 9999999999, self._treasureDurability,
            self._treasureDurability, {
            position = {
                x = 3 * TREASURE_SIZE / 2 + (3 + CENTER_X - FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
                y = START_Y + 3 * TREASURE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
                z = CENTER_Z * BLOCK_SIZE
            },
            rotation = {
                x = 0,
                y = 0,
                z = 0
            },
            scale = {
                x = 3 * TREASURE_SIZE,
                y = 3 * TREASURE_SIZE,
                z = 3 * TREASURE_SIZE
            }
        })
        scene.SetComponent(self._treasureEntities[2], "treasure", 9999999999, self._treasureDurability,
            self._treasureDurability, {
            position = {
                x = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_X + FINAL_LEVEL_WIDTH / 2) * BLOCK_SIZE,
                y = START_Y + 3 * TREASURE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
                z = CENTER_Z * BLOCK_SIZE
            },
            rotation = {
                x = 0,
                y = 0,
                z = 0
            },
            scale = {
                x = 3 * TREASURE_SIZE,
                y = 3 * TREASURE_SIZE,
                z = 3 * TREASURE_SIZE
            }
        })
        scene.SetComponent(self._treasureEntities[3], "treasure", 9999999999, self._treasureDurability,
            self._treasureDurability, {
            position = {
                x = CENTER_X * BLOCK_SIZE,
                y = START_Y + 3 * TREASURE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
                z = 3 * TREASURE_SIZE / 2 + (3 + CENTER_Z - FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
            },
            rotation = {
                x = 0,
                y = 0,
                z = 0
            },
            scale = {
                x = 3 * TREASURE_SIZE,
                y = 3 * TREASURE_SIZE,
                z = 3 * TREASURE_SIZE
            }
        })

        local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
        currentFuel = -1
        scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", currentFuel, maxFuel)

        scene.SetComponent(self._treasureEntities[4], "treasure", 9999999999, self._treasureDurability,
            self._treasureDurability, {
            position = {
                x = CENTER_X * BLOCK_SIZE,
                y = START_Y + 3 * TREASURE_SIZE / 2 - (1.5 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE,
                z = -3 * TREASURE_SIZE / 2 + (-3 + CENTER_Z + FINAL_LEVEL_HEIGHT / 2) * BLOCK_SIZE
            },
            rotation = {
                x = 0,
                y = 0,
                z = 0
            },
            scale = {
                x = 3 * TREASURE_SIZE,
                y = 3 * TREASURE_SIZE,
                z = 3 * TREASURE_SIZE
            }
        })

        while true do
            local now = os.clock()
            for _, mole in pairs(self._moles) do
                if not mole.alive and now - mole.timeKilled >= self._moleSpawningCooldown then
                    -- Spawn mole
                    mole.id = scene.CreateEntity()
                    mole.alive = true

                    local spawnPos = self._moleSpawnPoints[math.random(1, 4)]
                    spawnPos = {
                        x = spawnPos.x + (math.ult(0, math.random(0, 1)) and 1 or -1) * math.random(0, 4),
                        y = spawnPos.y,
                        z = spawnPos.z + (math.ult(0, math.random(0, 1)) and 1 or -1) * math.random(0, 4)
                    }

                    scene.SetComponent(mole.id, "mole", self._moleHealth, self._moleHealth, self._moleDamage, {
                        position = spawnPos,
                        rotation = {
                            x = 0,
                            y = 0,
                            z = 0
                        },
                        scale = self._bossMoleSize3
                    })
                    scene.SetComponent(mole.id, "behaviour", LUA_FILES_FOLDER .. "Mole.lua")
                elseif mole.alive and not scene.IsEntity(mole.id) then
                    -- Mole killed
                    mole.alive = false
                    mole.timeKilled = os.clock()
                end
            end
            for index, treasureEntity in ipairs(self._treasureEntities) do
                if not scene.IsEntity(treasureEntity) then
                    self._treasuresRemaining = self._treasuresRemaining - 1
                    self._moleHealth = 2000 + (self._maxMoles - self._treasuresRemaining) * 500
                    self._moleDamage = 30 + (self._maxMoles - self._treasuresRemaining) * 10
                    table.remove(self._treasureEntities, index)
                    if #self._treasureEntities == 0 then
                        PLAYER_WON = true
                    end
                end
            end
            coroutine.yield()
        end
    end)

    print("Boss room created!")
end

function bossRoom:OnUpdate(delta)
    local characterComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
    if characterComponent.transform.position.y < 3 * TREASURE_SIZE / 2 -
        (0.5 + characterComponent.transform.scale.y / 2 + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE then
        HAS_REACHED_BOSS_ROOM = true
        coroutine.resume(self._update)
    end
end

return bossRoom
