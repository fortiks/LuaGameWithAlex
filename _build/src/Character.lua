local character = {}

function character:OnCreate()
    self._reach = {
        block = 993.5,
        mole = 992.5,
        treasure = 993
    }
    self._lastSwing = os.clock()
    self._attacksPerSecond = 1.5
    self._speed = 5

    self._getRightVector = function(targetVector)
        return {
            x = -targetVector.z,
            y = 0,
            z = targetVector.x
        }
    end

    self._printPosition = function()
        print("[LUA] Character Position: (" ..
            tostring(self._transform.position.x) .. ",\t" ..
            tostring(self._transform.position.y) .. ")"
        )
    end

    if saves.tempCharacter then
        self._gold = saves.tempCharacter.gold
        self._damage = saves.tempCharacter.damage
        self._currentHealth = saves.tempCharacter.currentHealth
        self._maxHealth = saves.tempCharacter.maxHealth
        self._currentFuel = saves.tempCharacter.currentFuel
        self._maxFuel = saves.tempCharacter.maxFuel
        scene.SetComponent(self.ID, "health", self._currentHealth, self._maxHealth)
        scene.SetComponent(self.ID, "fuel", self._currentFuel, self._maxFuel)
        scene.SetComponent(self.ID, "backpack", saves.tempCharacter.maxWeight, saves.tempCharacter.currentWeight,
            saves.tempCharacter.dirtCount, saves.tempCharacter.stoneCount, saves.tempCharacter.bronzeCount,
            saves.tempCharacter.silverCount, saves.tempCharacter.goldenCount, saves.tempCharacter.diamondCount,
            saves.tempCharacter.ancientRelicCount)
    elseif saves.character then
        self._gold = saves.character.gold
        self._damage = saves.character.damage
        self._currentHealth = saves.character.currentHealth
        self._maxHealth = saves.character.maxHealth
        self._currentFuel = saves.character.currentFuel
        self._maxFuel = saves.character.maxFuel
        scene.SetComponent(self.ID, "health", self._currentHealth, self._maxHealth)
        scene.SetComponent(self.ID, "fuel", self._currentFuel, self._maxFuel)
        scene.SetComponent(self.ID, "backpack", saves.character.maxWeight, saves.character.currentWeight,
            saves.character.dirtCount, saves.character.stoneCount, saves.character.bronzeCount,
            saves.character.silverCount, saves.character.goldenCount, saves.character.diamondCount,
            saves.character.ancientRelicCount)
    else
        self._gold = 1000000 --403
        self._damage = 50000000
        self._currentHealth = 60 * 1000
        self._maxHealth = 100 * 1000
        self._currentFuel = 60 * 1000
        self._maxFuel = 100 * 1000
        scene.SetComponent(self.ID, "health", self._currentHealth, self._maxHealth)
        scene.SetComponent(self.ID, "fuel", self._currentFuel, self._maxFuel)
        scene.SetComponent(self.ID, "backpack", 25, 0, 0, 0, 0, 0, 0, 0, 0)
    end

    scene.SetComponent(self.ID, "character", self._gold, self._damage, {
        position = { x = -1, y = 4, z = -1 },
        rotation = { x = 0, y = 0, z = 0 },
        scale = { x = 1.5, y = 2, z = 1.0 }
    })
    scene.SetComponent(self.ID, "gravitation", -1.0, 10, false)
    print("Character created!")
end

function character:OnUpdate(delta)
    if scene.IsKeyPressed(string.byte("O")) then
        local blockMaterials = {}
        for _, blockEntity in ipairs(blockEntityIds) do
            local blockComponent = scene.GetComponent(blockEntity, "block")
            table.insert(blockMaterials, blockComponent.material)
        end

        local treasurePositions = {}
        for _, treasureEntity in ipairs(scene.GetEntities("treasure")) do
            local treasureComponent = scene.GetComponent(treasureEntity, "treasure")
            table.insert(treasurePositions, {
                x = (treasureComponent.transform.position.x - START_X) / BLOCK_SIZE,
                y = (START_Y - BLOCK_SIZE / 2 + TREASURE_SIZE / 2 - treasureComponent.transform.position.y) / BLOCK_SIZE,
                z = (treasureComponent.transform.position.z - START_Z) / BLOCK_SIZE
            })
        end

        local characterComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
        local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
        local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
        local backpackComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")

        scene.SaveTables({
            editorBlocks = blockMaterials,
            editorTreasures = treasurePositions,
            tempCharacter = {
                gold = characterComponent.gold,
                damage = characterComponent.damage,
                currentHealth = currentHealth,
                maxHealth = maxHealth,
                currentFuel = currentFuel,
                maxFuel = maxFuel,
                maxWeight = backpackComponent.maxWeight,
                currentWeight = backpackComponent.currentWeight,
                dirtCount = backpackComponent.dirtCount,
                stoneCount = backpackComponent.stoneCount,
                bronzeCount = backpackComponent.bronzeCount,
                silverCount = backpackComponent.silverCount,
                goldenCount = backpackComponent.goldenCount,
                diamondCount = backpackComponent.diamondCount,
                ancientRelicCount = backpackComponent.ancientRelicCount
            }
        })

        IS_IN_EDIT_MODE = true
    end

    local characterComponent = scene.GetComponent(self.ID, "character")
    local currentHealth, _ = scene.GetComponent(self.ID, "health")
    local now = os.clock()
    if scene.IsMouseButtonPressed(0) and now - self._lastSwing >= 1 / self._attacksPerSecond then
        self._lastSwing = os.clock()
        local entity, componentName, distance = scene.GetEntityAndComponentFromClick()

        -- Check for distance depending on componentName
        if componentName == "block" and distance <= self._reach.block then
            local blockComponent = scene.GetComponent(entity, "block")
            local blockKey = getMaterialNameFromId(blockComponent.material)
            local blockInfo = BLOCK_INFO[blockKey]
            if not (blockInfo.name == "stone" and characterComponent.damage <= 200) then
                local backpackComponent = scene.GetComponent(self.ID, "backpack")
                if backpackComponent.currentWeight + blockInfo.weight <= backpackComponent.maxWeight and
                    backpackComponent[blockInfo.name .. "Count"] ~= nil then
                    backpackComponent[blockInfo.name .. "Count"] = backpackComponent[blockInfo.name .. "Count"] + 1
                    backpackComponent.currentWeight = backpackComponent.currentWeight + blockComponent.weight
                    scene.SetComponent(self.ID, "backpack", backpackComponent.maxWeight, backpackComponent.currentWeight
                        ,
                        backpackComponent.dirtCount,
                        backpackComponent.stoneCount, backpackComponent.bronzeCount, backpackComponent.silverCount,
                        backpackComponent.goldCount, backpackComponent.diamondCount, backpackComponent.ancientRelicCount)
                end
                scene.SetComponent(entity, "block", blockComponent.gold, blockComponent.weight, blockComponent.position,
                    blockComponent.size, BLOCK_INFO.EMPTY.id)
            end
        elseif componentName == "mole" and distance <= self._reach.mole then
            local moleComponent = scene.GetComponent(entity, "mole")
            local moleHealth = moleComponent.health - characterComponent.damage
            if moleHealth < 0 then
                if not scene.HasComponent(entity, "treasure") then
                    scene.RemoveEntity(entity)
                else
                    scene.RemoveComponent(entity, "behaviour")
                    scene.RemoveComponent(entity, "mole")
                end
            else
                scene.SetComponent(entity, "mole", moleHealth, moleComponent.maxHealth, moleComponent.damage,
                    moleComponent.transform, true)
            end
        elseif componentName == "treasure" and distance <= self._reach.treasure then
            local treasureComponent = scene.GetComponent(entity, "treasure")
            local newTreasureDurability = treasureComponent.durability - characterComponent.damage
            if newTreasureDurability < 0 then
                characterComponent.gold = characterComponent.gold + treasureComponent.gold
                if not scene.HasComponent(entity, "mole") then
                    scene.RemoveEntity(entity)
                else
                    scene.RemoveComponent(entity, "treasure")
                end
            else
                scene.SetComponent(entity, "treasure", treasureComponent.gold, newTreasureDurability,
                    treasureComponent.maxDurability, treasureComponent.transform)
            end
        end
    end

    local currentFuel, maxFuel = scene.GetComponent(self.ID, "fuel")
    local transform = characterComponent.transform
    local moveDirectionXZ = {
        x = 0,
        z = 0
    }
    if collisionInfo.isColliding then
        collisionInfo.distanceToCollision = collisionInfo.distanceToCollision < 0.0001 and 0.0001 or
            collisionInfo.distanceToCollision
        collisionInfo.distanceToCollision = collisionInfo.distanceToCollision > BLOCK_SIZE / 32.0 and BLOCK_SIZE / 32.0
            or
            collisionInfo.distanceToCollision

        transform.position.x = transform.position.x + collisionInfo.normal.x * collisionInfo.distanceToCollision
        transform.position.y = transform.position.y + collisionInfo.normal.y * collisionInfo.distanceToCollision
        transform.position.z = transform.position.z + collisionInfo.normal.z * collisionInfo.distanceToCollision
        collisionInfo.isColliding = false
    else
        if scene.IsKeyDown(string.byte("W")) then
            moveDirectionXZ.x = moveDirectionXZ.x + transform.rotation.x
            moveDirectionXZ.z = moveDirectionXZ.z + transform.rotation.z
        end
        if scene.IsKeyDown(string.byte("S")) then
            moveDirectionXZ.x = moveDirectionXZ.x - transform.rotation.x
            moveDirectionXZ.z = moveDirectionXZ.z - transform.rotation.z
        end
        if scene.IsKeyDown(string.byte("A")) then
            local rightVector = self._getRightVector(transform.rotation)
            moveDirectionXZ.x = moveDirectionXZ.x - rightVector.x
            moveDirectionXZ.z = moveDirectionXZ.z - rightVector.z
        end
        if scene.IsKeyDown(string.byte("D")) then
            local rightVector = self._getRightVector(transform.rotation)
            moveDirectionXZ.x = moveDirectionXZ.x + rightVector.x
            moveDirectionXZ.z = moveDirectionXZ.z + rightVector.z
        end
        if scene.IsKeyDown(string.byte(" ")) and not HAS_REACHED_BOSS_ROOM then
            local gravity = scene.GetComponent(self.ID, "gravitation")
            if gravity.isTouchingGround then
                transform.position.y = transform.position.y + 0.05
                scene.SetComponent(self.ID, "gravitation", self._speed, 15, false)
            else
                scene.SetComponent(self.ID, "gravitation", gravity.velocityY + 5 * self._speed * delta,
                    gravity.gravityFactor, false)
            end
            currentFuel = currentFuel - self._speed * delta / 2
        end
    end
    local moveDirectionXZDistance = math.sqrt(moveDirectionXZ.x * moveDirectionXZ.x +
        moveDirectionXZ.z * moveDirectionXZ.z)
    transform.position.x = transform.position.x +
        (moveDirectionXZDistance == 0 and 0 or moveDirectionXZ.x / moveDirectionXZDistance) * self._speed * delta
    transform.position.z = transform.position.z +
        (moveDirectionXZDistance == 0 and 0 or moveDirectionXZ.z / moveDirectionXZDistance) * self._speed * delta

    currentFuel = currentFuel - self._speed * delta / 5

    if (currentFuel <= 0 and not HAS_REACHED_BOSS_ROOM) or (currentHealth <= 0) then
        -- Kill player
        PLAYER_ALIVE = false
    end

    scene.SetComponent(self.ID, "fuel", currentFuel, maxFuel)

    if transform.position.y - transform.scale.y / 2 <
        START_Y + transform.scale.y / 2 -
        (1.46 + FINAL_LEVEL_DEPTH + (#levels) * LEVEL_DEPTH) * BLOCK_SIZE and
        scene.HasComponent(self.ID, "gravitation") then

        scene.RemoveComponent(self.ID, "gravitation")
    end

    -- Determine which shop you are in
    for _, shop in pairs(shops) do
        local shopToCharacter = {
            x = transform.position.x - shop.worldTransform.position.x,
            z = transform.position.z - shop.worldTransform.position.z
        }
        local distanceToCharacter = math.sqrt(
            shopToCharacter.x * shopToCharacter.x +
            shopToCharacter.z * shopToCharacter.z
        )

        local shopComponent = scene.GetComponent(shop.id, "shop")
        if (
            distanceToCharacter < 2 * (shop.worldTransform.scale.x + shop.worldTransform.scale.z) / 2 and
                transform.position.y > -1.5 * BLOCK_SIZE) then
            scene.SetComponent(shopComponent.id, "shopUpdate", true)
        else
            scene.SetComponent(shopComponent.id, "shopUpdate", false)
        end
    end

    scene.SetComponent(self.ID, "character", characterComponent.gold, characterComponent.damage, transform)
end

return character
