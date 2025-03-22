shops = {
    survival = {
        id = scene.CreateEntity(),
        name = "Survival",
        open = false,
        color = {
            x = 200,
            y = 0,
            z = 0
        },
        frame = {
            posX = 0.5,
            posY = 0.5,
            width = 0.6,
            height = 0.6
        },
        worldTransform = {
            position = { x = CENTER_X * BLOCK_SIZE, y = 1.5, z = (CENTER_Z + HEIGHT / 2 + 5 + 1 / 2) * BLOCK_SIZE },
            rotation = { x = 0, y = 0, z = 0 },
            scale = { x = 7, y = 9, z = 7 }
        }
    },
    upgrade = {
        id = scene.CreateEntity(),
        name = "Upgrade",
        open = false,
        color = {
            x = 0,
            y = 0,
            z = 200
        },
        frame = {
            posX = 0.5,
            posY = 0.5,
            width = 0.6,
            height = 0.6
        },
        worldTransform = {
            position = { x = CENTER_X * BLOCK_SIZE, y = 1.5, z = (CENTER_Z - HEIGHT / 2 - 5 - 1 / 2) * BLOCK_SIZE },
            rotation = { x = 0, y = 0, z = 0 },
            scale = { x = 7, y = 9, z = 7 }
        }
    },
    sell = {
        id = scene.CreateEntity(),
        name = "sell",
        open = false,
        color = {
            x = 225,
            y = 225,
            z = 0
        },
        frame = {
            posX = 0.5,
            posY = 0.5,
            width = 0.6,
            height = 0.6
        },
        worldTransform = {
            position = { x = (CENTER_X - WIDTH / 2 - 5 - 1 / 2) * BLOCK_SIZE, y = 1.5, z = (CENTER_Z) * BLOCK_SIZE },
            rotation = { x = 0, y = 0, z = 0 },
            scale = { x = 7, y = 9, z = 7 }
        }
    },
    save = {
        id = scene.CreateEntity(),
        name = "save",
        open = false,
        color = {
            x = 200,
            y = 122,
            z = 255
        },
        frame = {
            posX = 0.5,
            posY = 0.5,
            width = 0.6,
            height = 0.6
        },
        worldTransform = {
            position = { x = (CENTER_X + WIDTH / 2 + 5 + 1 / 2) * BLOCK_SIZE, y = 1.5, z = (CENTER_Z) * BLOCK_SIZE },
            rotation = { x = 0, y = 0, z = 0 },
            scale = { x = 7, y = 9, z = 7 }
        }
    }
}

shops.survival.textLabels = {}
shops.survival.onOpen = function() end
shops.survival.buttons = {
    heal10 = {
        name = "Health 10+",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 100 gold"
        end,
        posX = 0.2,
        posY = 0.2,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 100 then
                local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
                if currentHealth == maxHealth then
                    return
                end
                currentHealth = maxHealth - currentHealth >= 10 and currentHealth + 10 or
                    currentHealth + (maxHealth - currentHealth)
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", currentHealth, maxHealth)
                character.gold = character.gold - 100
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    heal25 = {
        name = "Health 25+",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 250 gold"
        end,
        posX = 0.2,
        posY = 0.4,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 250 then
                local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
                if currentHealth == maxHealth then
                    return
                end
                currentHealth = maxHealth - currentHealth >= 25 and currentHealth + 25 or
                    currentHealth + (maxHealth - currentHealth)
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", currentHealth, maxHealth)
                character.gold = character.gold - 250
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    heal50 = {
        name = "Health 50+",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 500 gold"
        end,
        posX = 0.2,
        posY = 0.6,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 500 then
                local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
                if currentHealth == maxHealth then
                    return
                end
                currentHealth = maxHealth - currentHealth >= 50 and currentHealth + 50 or
                    currentHealth + (maxHealth - currentHealth)
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", currentHealth, maxHealth)
                character.gold = character.gold - 500
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    heal75 = {
        name = "Health 75+",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 750 gold"
        end,
        posX = 0.2,
        posY = 0.8,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 750 then
                local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
                if currentHealth == maxHealth then
                    return
                end
                currentHealth = maxHealth - currentHealth >= 10 and currentHealth + 10 or
                    currentHealth + (maxHealth - currentHealth)
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", currentHealth, maxHealth)
                character.gold = character.gold - 750
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    healFull = {
        name = "Full Health",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 1000 gold"
        end,
        posX = 0.5,
        posY = 0.4,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 1000 then
                local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
                if currentHealth == maxHealth then
                    return
                end
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", maxHealth, maxHealth)
                character.gold = character.gold - 1000
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    fillTank = {
        name = "Fill Tank",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 500 gold"
        end,
        posX = 0.5,
        posY = 0.6,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 500 then
                local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
                if currentFuel == maxFuel then
                    return
                end
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", maxFuel, maxFuel)
                character.gold = character.gold - 500
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    fill75 = {
        name = "Fill 75",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 375 gold"
        end,
        posX = 0.8,
        posY = 0.8,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 375 then
                local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
                if currentFuel == maxFuel then
                    return
                end
                currentFuel = maxFuel - currentFuel >= 375 and currentFuel + 375 or
                    currentFuel + (maxFuel - currentFuel
                    )
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", currentFuel, maxFuel)
                character.gold = character.gold - 375
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    fill50 = {
        name = "Fill half a Tank",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 250 gold"
        end,
        posX = 0.8,
        posY = 0.6,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 250 then
                local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
                if currentFuel == maxFuel then
                    return
                end
                currentFuel = maxFuel - currentFuel >= 250 and currentFuel + 250 or
                    currentFuel + (maxFuel - currentFuel
                    )
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", currentFuel, maxFuel)
                character.gold = character.gold - 250
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    fill25 = {
        name = "Fill 25",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 125 gold"
        end,
        posX = 0.8,
        posY = 0.4,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 125 then
                local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
                if currentFuel == maxFuel then
                    return
                end
                currentFuel = maxFuel - currentFuel >= 125 and currentFuel + 125 or
                    currentFuel + (maxFuel - currentFuel
                    )
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", currentFuel, maxFuel)
                character.gold = character.gold - 125
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    },
    Tank10 = {
        name = "Fill 10",
        shopId = shops.survival.id,
        tooltip = function()
            return "Cost 50 gold"
        end,
        posX = 0.8,
        posY = 0.2,
        width = 0.128,
        height = 0.072,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if character.gold >= 50 then
                local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
                if currentFuel == maxFuel then
                    return
                end
                currentFuel = maxFuel - currentFuel >= 50 and currentFuel + 50 or currentFuel + (maxFuel - currentFuel)
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", currentFuel, maxFuel)
                character.gold = character.gold - 50
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
            end
        end
    }
}

shops.upgrade.textLabels = {
    damage = {
        id = scene.CreateEntity(),
        nameBlueprint = "Player damage: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.25,
        width = 0.5,
        height = 0.15
    },
    maxHealth = {
        id = scene.CreateEntity(),
        nameBlueprint = "Player max health: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.45,
        width = 0.5,
        height = 0.15
    },
    maxFuel = {
        id = scene.CreateEntity(),
        nameBlueprint = "Max fuel: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.65,
        width = 0.5,
        height = 0.15
    },
    maxWeight = {
        id = scene.CreateEntity(),
        nameBlueprint = "Player backpack capacity: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.85,
        width = 0.5,
        height = 0.15
    }
}
shops.upgrade.onOpen = function()
    shops.upgrade.textLabels.damage.displayName = string.format(shops.upgrade.textLabels.damage.nameBlueprint,
        scene.GetComponent(CHARACTER_ENTITY_ID, "character").damage)
    local damageLabel = scene.GetComponent(shops.upgrade.textLabels.damage.id, "textLabel")
    scene.SetComponent(damageLabel.id, "textLabel", shops.upgrade.textLabels.damage.displayName, damageLabel.id,
        shops.upgrade.id, damageLabel.screenPointX, damageLabel.screenPointY, damageLabel.screenWidth,
        damageLabel.screenHeight)
    damageLabel = scene.GetComponent(shops.upgrade.textLabels.damage.id, "textLabel")

    shops.upgrade.textLabels.maxFuel.displayName = string.format(shops.upgrade.textLabels.maxFuel.nameBlueprint,
        select(2, scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")))
    local maxFuelLabel = scene.GetComponent(shops.upgrade.textLabels.maxFuel.id, "textLabel")
    scene.SetComponent(maxFuelLabel.id, "textLabel", shops.upgrade.textLabels.maxFuel.displayName, maxFuelLabel.id,
        shops.upgrade.id, maxFuelLabel.screenPointX, maxFuelLabel.screenPointY, maxFuelLabel.screenWidth,
        maxFuelLabel.screenHeight)

    shops.upgrade.textLabels.maxHealth.displayName = string.format(shops.upgrade.textLabels.maxHealth.nameBlueprint,
        select(2, scene.GetComponent(CHARACTER_ENTITY_ID, "health")))
    local maxHealthLabel = scene.GetComponent(shops.upgrade.textLabels.maxHealth.id, "textLabel")
    scene.SetComponent(maxHealthLabel.id, "textLabel", shops.upgrade.textLabels.maxHealth.displayName, maxHealthLabel.id
        ,
        shops.upgrade.id, maxHealthLabel.screenPointX, maxHealthLabel.screenPointY, maxHealthLabel.screenWidth,
        maxHealthLabel.screenHeight)

    shops.upgrade.textLabels.maxWeight.displayName = string.format(shops.upgrade.textLabels.maxWeight.nameBlueprint,
        scene.GetComponent(CHARACTER_ENTITY_ID, "backpack").maxWeight)
    local maxWeightLabel = scene.GetComponent(shops.upgrade.textLabels.maxWeight.id, "textLabel")
    scene.SetComponent(maxWeightLabel.id, "textLabel", shops.upgrade.textLabels.maxWeight.displayName, maxWeightLabel.id
        ,
        shops.upgrade.id, maxWeightLabel.screenPointX, maxWeightLabel.screenPointY, maxWeightLabel.screenWidth,
        maxWeightLabel.screenHeight)
end
shops.upgrade.buttons = {
    drill = {
        name = "Upgrade drill damage",
        shopId = shops.upgrade.id,
        tooltip = function()
            local damage = scene.GetComponent(CHARACTER_ENTITY_ID, "character").damage
            local message
            if (damage == 10) then
                message = "Cost 1000 gold"
            elseif (damage == 25) then
                message = "Cost 5000 gold"
            elseif (damage == 50) then
                message = "Cost 10000 gold"
            elseif (damage == 75) then
                message = "Cost 25000 gold"
            elseif (damage == 100) then
                message = "Cost 1000000 gold"
            elseif (damage == 1000) then
                message = "Max upgraded"
            end
            return message
        end,
        posX = 0.2,
        posY = 0.2,
        width = 0.2,
        height = 0.08,
        onClick = function()
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if (character.damage == 10 and character.gold >= 1000) then
                character.damage = 25
                character.gold = character.gold - 1000
            elseif (character.damage == 25 and character.gold >= 5000) then
                character.damage = 50
                character.gold = character.gold - 5000
            elseif (character.damage == 50 and character.gold >= 10000) then
                character.damage = 75
                character.gold = character.gold - 10000
            elseif (character.damage == 75 and character.gold >= 25000) then
                character.damage = 100
                character.gold = character.gold - 25000
            elseif (character.damage == 100 and character.gold >= 100000) then
                character.damage = 1000
                character.gold = character.gold - 100000
            end
            scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage, character.transform)
        end
    },
    uppgradeHealth = {
        name = "Upgrade health",
        shopId = shops.upgrade.id,
        tooltip = function()
            local _, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
            local message
            if (maxHealth == 100) then
                message = "Cost 1000 gold"
            elseif (maxHealth == 125) then
                message = "Cost 5000 gold"
            elseif (maxHealth == 150) then
                message = "Cost 10000 gold"
            elseif (maxHealth == 175) then
                message = "Cost 25000 gold"
            elseif (maxHealth == 200) then
                message = "Cost 1000000 gold"
            elseif (maxHealth == 250) then
                message = "Max upgraded"
            end
            return message
        end,
        posX = 0.2,
        posY = 0.4,
        width = 0.2,
        height = 0.08,
        onClick = function()
            local _, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")

            local newMaxHealth = maxHealth
            if (maxHealth == 100 and character.gold >= 1000) then
                newMaxHealth = 125
                character.gold = character.gold - 1000
            elseif (maxHealth == 125 and character.gold >= 5000) then
                newMaxHealth = 150
                character.gold = character.gold - 5000
            elseif (maxHealth == 150 and character.gold >= 10000) then
                newMaxHealth = 175
                character.gold = character.gold - 10000
            elseif (maxHealth == 175 and character.gold >= 25000) then
                newMaxHealth = 200
                character.gold = character.gold - 25000
            elseif (maxHealth == 200 and character.gold >= 100000) then
                newMaxHealth = 250
                character.gold = character.gold - 100000
            end

            if maxHealth ~= newMaxHealth then
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
                scene.SetComponent(CHARACTER_ENTITY_ID, "health", newMaxHealth, newMaxHealth)
            end
        end
    },
    uppgradeFuel = {
        name = "Upgrade fuel capacity",
        shopId = shops.upgrade.id,
        tooltip = function()
            local _, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
            local message
            if (maxFuel == 100) then
                message = "Cost 1000 gold"
            elseif (maxFuel == 125) then
                message = "Cost 5000 gold"
            elseif (maxFuel == 150) then
                message = "Cost 10000 gold"
            elseif (maxFuel == 175) then
                message = "Cost 25000 gold"
            elseif (maxFuel == 200) then
                message = "Cost 1000000 gold"
            elseif (maxFuel == 250) then
                message = "Max upgraded"
            end
            return message
        end,
        posX = 0.2,
        posY = 0.6,
        width = 0.2,
        height = 0.08,
        onClick = function()
            local _, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")

            local newMaxFuel = maxFuel
            if (maxFuel == 100 and character.gold >= 1000) then
                newMaxFuel = 125
                character.gold = character.gold - 1000
            elseif (maxFuel == 125 and character.gold >= 5000) then
                newMaxFuel = 150
                character.gold = character.gold - 5000
            elseif (maxFuel == 150 and character.gold >= 10000) then
                newMaxFuel = 175
                character.gold = character.gold - 10000
            elseif (maxFuel == 175 and character.gold >= 25000) then
                newMaxFuel = 200
                character.gold = character.gold - 25000
            elseif (maxFuel == 200 and character.gold >= 100000) then
                newMaxFuel = 250
                character.gold = character.gold - 100000
            end

            if maxFuel ~= newMaxFuel then
                scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage,
                    character.transform)
                scene.SetComponent(CHARACTER_ENTITY_ID, "fuel", newMaxFuel, newMaxFuel)
            end
        end
    },
    uppgradeInventory = {
        name = "Upgrade inventory space",
        shopId = shops.upgrade.id,
        tooltip = function()
            local backpack = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")
            local message
            if (backpack.maxWeight == 25) then
                message = "Cost 1000 gold"
            elseif (backpack.maxWeight == 50) then
                message = "Cost 5000 gold"
            elseif (backpack.maxWeight == 75) then
                message = "Cost 10000 gold"
            elseif (backpack.maxWeight == 100) then
                message = "Cost 25000 gold"
            elseif (backpack.maxWeight == 150) then
                message = "Cost 1000000 gold"
            elseif (backpack.maxWeight == 250) then
                message = "Max upgraded"
            end
            return message
        end,
        posX = 0.2,
        posY = 0.8,
        width = 0.2,
        height = 0.08,
        onClick = function()
            local backpack = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")
            local character = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            if (backpack.maxWeight == 25 and character.gold >= 1000) then
                backpack.maxWeight = 50
                character.gold = character.gold - 1000
            elseif (backpack.maxWeight == 50 and character.gold >= 5000) then
                backpack.maxWeight = 75
                character.gold = character.gold - 5000
            elseif (backpack.maxWeight == 75 and character.gold >= 10000) then
                backpack.maxWeight = 100
                character.gold = character.gold - 10000
            elseif (backpack.maxWeight == 100 and character.gold >= 25000) then
                backpack.maxWeight = 150
                character.gold = character.gold - 25000
            elseif (backpack.maxWeight == 150 and character.gold >= 100000) then
                backpack.maxWeight = 250
                character.gold = character.gold - 100000
            end

            scene.SetComponent(CHARACTER_ENTITY_ID, "character", character.gold, character.damage, character.transform)
            scene.SetComponent(CHARACTER_ENTITY_ID, "backpack", backpack.maxWeight, backpack.currentWeight,
                backpack.dirtCount, backpack.stoneCount, backpack.bronzeCount, backpack.silverCount,
                backpack.goldenCount, backpack.diamondCount, backpack.ancientRelicCount)
        end
    }
}

shops.sell.textLabels = {
    dirt = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of dirt: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.2,
        width = 0.5,
        height = 0.08
    },
    stone = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of stone: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.32,
        width = 0.5,
        height = 0.08
    },
    bronze = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of bronze ore: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.44,
        width = 0.5,
        height = 0.08
    },
    silver = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of silver ore: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.56,
        width = 0.5,
        height = 0.08
    },
    golden = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of gold ore: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.68,
        width = 0.5,
        height = 0.08
    },
    diamond = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of diamonds: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.80,
        width = 0.5,
        height = 0.08
    },
    ancientRelic = {
        id = scene.CreateEntity(),
        nameBlueprint = "Number of ancient relics: %d",
        displayName = "",
        posX = 1.0,
        posY = 0.92,
        width = 0.5,
        height = 0.08
    },
}
shops.sell.onOpen = function()
    local backpackComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")
    for name, textLabel in pairs(shops.sell.textLabels) do
        textLabel.displayName = string.format(textLabel.nameBlueprint, backpackComponent[name .. "Count"])
        scene.SetComponent(textLabel.id, "textLabel", textLabel.displayName, textLabel.id, shops.sell.id,
            textLabel.posX, textLabel.posY, textLabel.width, textLabel.height)
    end
end
shops.sell.buttons = {
    sell = {
        name = "Sell",
        shopId = shops.sell.id,
        tooltip = function()
            return "Sell inventory"
        end,
        posX = 0.2,
        posY = 0.6,
        width = 0.20,
        height = 0.15,
        onClick = function()
            local characterComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            local backpackComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")

            local sum = characterComponent.gold
            for blockMaterial, blockInfo in pairs(BLOCK_INFO) do
                local name = BLOCK_INFO[blockMaterial].name .. "Count"
                if backpackComponent[name] ~= nil then
                    sum = sum + backpackComponent[name] * BLOCK_INFO[blockMaterial].value
                end
            end

            scene.SetComponent(CHARACTER_ENTITY_ID, "character", sum, characterComponent.damage,
                characterComponent.transform)
            scene.SetComponent(CHARACTER_ENTITY_ID, "backpack", backpackComponent.maxWeight, 0, 0, 0, 0, 0, 0, 0, 0)
        end
    }
}

shops.save.textLabels = {}
shops.save.buttons = {
    sell = {
        name = "Save Game",
        shopId = shops.save.id,
        tooltip = function()
            return "Save game state"
        end,
        posX = 0.5,
        posY = 0.6,
        width = 0.20,
        height = 0.15,
        onClick = function()
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
                    y = (START_Y - BLOCK_SIZE / 2 + TREASURE_SIZE / 2 - treasureComponent.transform.position.y) /
                        BLOCK_SIZE,
                    z = (treasureComponent.transform.position.z - START_Z) / BLOCK_SIZE
                })
            end

            local characterComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
            local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
            local currentFuel, maxFuel = scene.GetComponent(CHARACTER_ENTITY_ID, "fuel")
            local backpackComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "backpack")

            scene.SaveTables({
                blocks = blockMaterials,
                treasures = treasurePositions,
                character = {
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
        end
    }
}
shops.save.onOpen = function() end

for _, shop in pairs(shops) do
    local frame = shop.frame
    scene.SetComponent(shop.id, "shop", shop.id, shop.name, frame.posX, frame.posY, frame.width, frame.height, shop.open
        ,
        shop.worldTransform, shop.color, shop.onOpen)
    for _, textLabel in pairs(shop.textLabels) do
        scene.SetComponent(textLabel.id, "textLabel", textLabel.displayName, textLabel.id, shop.id, textLabel.posX,
            textLabel.posY, textLabel.width, textLabel.height)
    end
    for _, button in pairs(shop.buttons) do
        scene.SetComponent(scene.CreateEntity(), "button", button.name, button.shopId, button.tooltip, button.posX,
            button.posY, button.width, button.height, button.onClick)
    end
end
