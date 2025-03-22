local editor = {}

function editor:OnCreate()
    self._realBlocks = {}
    self._realTreasures = {}
    self._selected = {
        level = nil,
        slice = nil,
        block = nil
    }
    self._textLabels = {

    }

    self._boundingBoxes = {
        levels = {},
        slices = {},
        blocks = {}
    }
    self._selectedBoundingBoxes = {}

    self._guis = {
        editorControls = {
            rectangles = {
                {
                    id = nil,
                    posX = 0.0,
                    posY = 0.8,
                    sizeX = 0.25,
                    sizeY = 0.2,
                    color = {
                        x = 146,
                        y = 150,
                        z = 141
                    }
                }
            },
            textLabels = {
                {
                    id = nil,
                    posX = 0.01,
                    posY = 0.83,
                    text = "B - Go back"
                },
                {
                    id = nil,
                    posX = 0.01,
                    posY = 0.88,
                    text = "Z - Go back to playing"
                },
                {
                    id = nil,
                    posX = 0.01,
                    posY = 0.93,
                    text = "ENTER - Save map"
                }
            }
        },
        selectedControls = {
            rectangles = {
                {
                    id = nil,
                    posX = 0,
                    posY = 0,
                    sizeX = 0.25,
                    sizeY = 0.5,
                    color = {
                        x = 146,
                        y = 150,
                        z = 141
                    }
                }
            },
            textLabels = {
                noSelected = {
                    {
                        id = nil,
                        posX = 0.06,
                        posY = 0.03,
                        text = "No level selected"
                    },
                    {
                        id = nil,
                        posX = 0.01,
                        posY = 0.08,
                        text = "R - Randomize map"
                    }
                },
                levelSelect = {
                    {
                        id = nil,
                        posX = 0.06,
                        posY = 0.13,
                        text = "Level selected"
                    },
                    {
                        id = nil,
                        posX = 0.01,
                        posY = 0.18,
                        text = "R - Randomize level"
                    }
                },
                sliceSelect = {
                    {
                        id = nil,
                        posX = 0.06,
                        posY = 0.23,
                        text = "Slice selected"
                    },
                    {
                        id = nil,
                        posX = 0.01,
                        posY = 0.28,
                        text = "R - Randomize slice"
                    }
                },
                blockSelect = {
                    {
                        id = nil,
                        posX = 0.06,
                        posY = 0.33,
                        text = "Block selected"
                    },
                    {
                        id = nil,
                        posX = 0.01,
                        posY = 0.38,
                        text = "Q and E - Change material"
                    },
                    {
                        id = nil,
                        posX = 0.01,
                        posY = 0.43,
                        text = "G - Create room (needs space)"
                    }
                }
            }
        }
    }

    for _, gui in pairs(self._guis) do
        for _, rect in pairs(gui.rectangles) do
            rect.id = scene.CreateEntity()
            scene.SetComponent(rect.id, "guiRec", rect.posX, rect.posY, rect.sizeX, rect.sizeY, rect.color)
        end
    end

    for _, textLabel in pairs(self._guis.editorControls.textLabels) do
        textLabel.id = scene.CreateEntity()
        scene.SetComponent(textLabel.id, "guiTextLabel", textLabel.posX, textLabel.posY, textLabel.text)
    end


    for _, state in pairs(self._guis.selectedControls.textLabels) do
        for _, textLabel in pairs(state) do
            textLabel.id = scene.CreateEntity()
            scene.SetComponent(textLabel.id, "guiTextLabel", textLabel.posX, textLabel.posY, textLabel.text)
        end
    end

    for level = 1, #levels do
        table.insert(self._realTreasures, level, {})
    end

    self._generateBoundingBoxes = function()
        for levelNr, level in ipairs(levels) do
            for depth = 0, LEVEL_DEPTH - 1 do
                table.insert(self._boundingBoxes.slices, (levelNr - 1) * LEVEL_DEPTH + depth + 1, {
                    min = {
                        x = START_X - BLOCK_SIZE / 2,
                        y = -(level.beginningDepth + depth) * BLOCK_SIZE,
                        z = START_Z - BLOCK_SIZE / 2
                    },
                    max = {
                        x = WIDTH * BLOCK_SIZE,
                        y = START_Y - BLOCK_SIZE + BLOCK_SIZE / 2 - (level.beginningDepth + depth) * BLOCK_SIZE,
                        z = HEIGHT * BLOCK_SIZE
                    }
                })
            end
            table.insert(self._boundingBoxes.levels, levelNr, {
                min = {
                    x = 0,
                    y = START_Y + BLOCK_SIZE / 2 - level.beginningDepth * BLOCK_SIZE,
                    z = 0
                },
                max = {
                    x = WIDTH * BLOCK_SIZE,
                    y = START_Y - BLOCK_SIZE / 2 - level.depth * BLOCK_SIZE,
                    z = HEIGHT * BLOCK_SIZE
                }
            })
        end

        for blockNr, blockEntity in ipairs(blockEntityIds) do
            local blockComponent = scene.GetComponent(blockEntity, "block")
            table.insert(self._boundingBoxes.blocks, blockNr, {
                min = {
                    x = blockComponent.position.x - BLOCK_SIZE / 2,
                    y = blockComponent.position.y + BLOCK_SIZE / 2,
                    z = blockComponent.position.z - BLOCK_SIZE / 2
                },
                max = {
                    x = blockComponent.position.x + BLOCK_SIZE / 2,
                    y = blockComponent.position.y - BLOCK_SIZE / 2,
                    z = blockComponent.position.z + BLOCK_SIZE / 2
                }
            })

            table.insert(self._realBlocks, blockNr, blockComponent.material)
        end
    end
    self._findClosestBoundingBox = function(ray, boundingBoxes)
        local blocksHit = {}
        for levelNr, boundingBox in ipairs(boundingBoxes) do
            local rayCollision = scene.GetRayCollisionBox(ray, {
                min = boundingBox.min,
                max = boundingBox.max
            })

            if rayCollision.hit then
                table.insert(blocksHit, {
                    rayCollision = rayCollision,
                    levelNr = levelNr
                })
            end
        end

        local next = next
        if next(blocksHit) == nil then
            return nil, nil
        else
            table.sort(blocksHit, function(hit1, hit2)
                return hit1.rayCollision.distance < hit2.rayCollision.distance
            end)

            local closestLevelNr = blocksHit[1].levelNr
            local closestBoundingBox = boundingBoxes[closestLevelNr]

            return closestLevelNr, closestBoundingBox
        end
    end
    self._selectBoundingBox = function(targetedBoundingBox)
        local boxDimensions = {
            x = math.abs(targetedBoundingBox.min.x - targetedBoundingBox.max.x),
            y = math.abs(targetedBoundingBox.min.y - targetedBoundingBox.max.y),
            z = math.abs(targetedBoundingBox.min.z - targetedBoundingBox.max.z)
        }

        TARGET_BOUNDING_BOX = {
            center = {
                x = targetedBoundingBox.min.x + boxDimensions.x / 2,
                y = targetedBoundingBox.min.y - boxDimensions.y / 2,
                z = targetedBoundingBox.min.z + boxDimensions.z / 2
            },
            size = {
                x = boxDimensions.x + 0.05,
                y = boxDimensions.y + 0.05,
                z = boxDimensions.z + 0.05
            }
        }
    end
    self._removeTreasuresInSlice = function(levelNr, sliceNr)
        local treasuresToRemove = {}
        for _, treasure in ipairs(self._realTreasures[levelNr]) do
            if (treasure.position.y == sliceNr) then
                table.insert(treasuresToRemove, treasure.id)
            end
        end

        for _, entity in pairs(treasuresToRemove) do
            for treasureIndex, treasure in ipairs(self._realTreasures[levelNr]) do
                if (entity == treasure.id) then
                    scene.RemoveEntity(treasure.id)
                    table.remove(self._realTreasures[levelNr], treasureIndex)
                end
            end
        end
    end
    self._hideTreasuresInSlice = function(levelNr, sliceNr)
        for _, treasure in ipairs(self._realTreasures[levelNr]) do
            if (treasure.position.y == sliceNr) then
                local treasureComponent = scene.GetComponent(treasure.id, "treasure")
                treasureComponent.transform.scale = {
                    x = 0,
                    y = 0,
                    z = 0
                }
                scene.SetComponent(treasure.id, "treasure", treasureComponent.value, treasureComponent.durability,
                    treasureComponent.maxDurability, treasureComponent.transform)
            end
        end
    end
    self._showTreasuresInSlice = function(levelNr, sliceNr)
        for _, treasure in ipairs(self._realTreasures[levelNr]) do
            if (treasure.position.y == sliceNr) then
                local treasureComponent = scene.GetComponent(treasure.id, "treasure")
                treasureComponent.transform.scale = {
                    x = TREASURE_SIZE,
                    y = TREASURE_SIZE,
                    z = TREASURE_SIZE
                }
                scene.SetComponent(treasure.id, "treasure", treasureComponent.value, treasureComponent.durability,
                    treasureComponent.maxDurability, treasureComponent.transform)
            end
        end
    end
    self._removeSlices = function(beforeStartIndex, beforeLastIndex, afterStartIndex, afterLastIndex)
        -- Remove blocks between beforeStartIndex and beforeLastIndex
        for i = beforeStartIndex, beforeLastIndex do
            local blockEntity = blockEntityIds[i]
            local blockComponent = scene.GetComponent(blockEntity, "block")
            scene.SetComponent(blockEntity, "block", BLOCK_INFO.EMPTY.value, BLOCK_INFO.EMPTY.value,
                blockComponent.position, blockComponent.size, BLOCK_INFO.EMPTY.id)
        end

        -- Remove blocks between afterStartIndex and afterLastIndex
        for i = afterStartIndex, afterLastIndex do
            local blockEntity = blockEntityIds[i]
            local blockComponent = scene.GetComponent(blockEntity, "block")
            scene.SetComponent(blockEntity, "block", BLOCK_INFO.EMPTY.value, BLOCK_INFO.EMPTY.value,
                blockComponent.position, blockComponent.size, BLOCK_INFO.EMPTY.id)
        end

        -- Hide treasures before and after slice
        for y = math.floor((beforeStartIndex - WIDTH * HEIGHT) / (WIDTH * HEIGHT)) + 1, math.floor((beforeLastIndex -
            WIDTH * HEIGHT) / (WIDTH * HEIGHT)) do
            local levelNr = math.floor((y - 1) / LEVEL_DEPTH + 1)
            local sliceNr = y - (levelNr - 1) * LEVEL_DEPTH
            self._hideTreasuresInSlice(levelNr, sliceNr)
        end

        for y = math.floor((afterStartIndex - WIDTH * HEIGHT) / (WIDTH * HEIGHT)) + 1, math.floor((
            afterLastIndex - WIDTH * HEIGHT) / (WIDTH * HEIGHT)) do
            local levelNr = math.floor((y - 1) / LEVEL_DEPTH + 1)
            local sliceNr = y - (levelNr - 1) * LEVEL_DEPTH
            self._hideTreasuresInSlice(levelNr, sliceNr)
        end
    end

    scene.LoadTables({
        "editorBlocks",
        "editorTreasures"
    })

    if not saves.editorBlocks or not saves.editorTreasures then
        for z = 0, HEIGHT - 1 do
            for x = 0, WIDTH - 1 do
                local blockEntityId = scene.CreateEntity()
                table.insert(blockEntityIds, blockEntityId)
                -- table.insert(self._realBlockIds, BLOCK_INFO.EMPTY.id)
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
        for _, level in ipairs(levels) do
            for y = level.beginningDepth, level.depth do
                for z = 0, HEIGHT - 1 do
                    for x = 0, WIDTH - 1 do
                        local blockEntityId = scene.CreateEntity()
                        -- local value, weight, chosenMaterial = chooseMaterial(level.probabilities)
                        table.insert(blockEntityIds, blockEntityId)
                        -- table.insert(self._realBlockIds, BLOCK_INFO.DIRT.id)
                        scene.SetComponent(blockEntityId, "block", BLOCK_INFO.DIRT.value, BLOCK_INFO.DIRT.weight, {
                            x = START_X + x * BLOCK_SIZE,
                            y = START_Y - BLOCK_SIZE * y,
                            z = START_Z + z * BLOCK_SIZE
                        }, {
                            x = BLOCK_SIZE,
                            y = BLOCK_SIZE,
                            z = BLOCK_SIZE
                        }, BLOCK_INFO.DIRT.id
                        )
                    end
                end
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
    else
        for blockNr, blockMaterial in ipairs(saves.editorBlocks) do
            local blockEntity = scene.CreateEntity()
            table.insert(blockEntityIds, blockEntity)
            local materialName = getMaterialNameFromId(blockMaterial) or BLOCK_INFO.DIRT.name
            local key = string.upper(materialName)
            scene.SetComponent(blockEntity, "block", BLOCK_INFO[key].value, BLOCK_INFO[key].weight, {
                x = START_X + ((blockNr - 1) % WIDTH) * BLOCK_SIZE,
                y = START_Y - math.floor((blockNr - 1) / (WIDTH * HEIGHT)) * BLOCK_SIZE,
                z = START_Z + math.floor(((blockNr - 1) % (WIDTH * HEIGHT)) / WIDTH) * BLOCK_SIZE
            }, {
                x = BLOCK_SIZE,
                y = BLOCK_SIZE,
                z = BLOCK_SIZE
            }, BLOCK_INFO[key].id
            )
        end

        for _, treasurePosition in pairs(saves.editorTreasures) do
            local treasureEntity = scene.CreateEntity()
            local levelNr = math.floor((treasurePosition.y - 1) / LEVEL_DEPTH) + 1
            scene.SetComponent(treasureEntity, "treasure", levelNr * levelNr * 8000, levelNr * levelNr * 100,
                levelNr * levelNr * 100, {
                position = {
                    x = START_X + treasurePosition.x * BLOCK_SIZE,
                    y = START_Y - treasurePosition.y * BLOCK_SIZE - BLOCK_SIZE / 2 +
                        TREASURE_SIZE / 2,
                    z = START_Z + treasurePosition.z * BLOCK_SIZE
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

            local levelNr = math.floor((treasurePosition.y - 1) / LEVEL_DEPTH) + 1
            local sliceNr = treasurePosition.y - (levelNr - 1) * LEVEL_DEPTH
            table.insert(self._realTreasures[levelNr], {
                id = treasureEntity,
                position = {
                    x = treasurePosition.x,
                    y = sliceNr,
                    z = treasurePosition.z
                },
            })
        end
    end

    self._generateBoundingBoxes()
    self._selectedBoundingBoxes = self._boundingBoxes.levels
    IS_IN_EDIT_MODE = true

    print("Editor created")
end

function editor:OnUpdate()
    -- Create a bounding box for target
    local ray = scene.GetRayCenter()
    local index, targetedBoundingBox = self._findClosestBoundingBox(ray, self._selectedBoundingBoxes)
    if targetedBoundingBox then
        if not self._selected.block then
            self._selectBoundingBox(targetedBoundingBox)
        end

        if scene.IsMouseButtonPressed(0) then
            if not self._selected.level then
                self._selected.level = index
                self._selectedBoundingBoxes = {}
                for sliceNr = 1, LEVEL_DEPTH do
                    table.insert(self._selectedBoundingBoxes, sliceNr,
                        self._boundingBoxes.slices[LEVEL_DEPTH * (index - 1) + sliceNr])
                end
                local afterStartIndex = self._selected.level * WIDTH * HEIGHT * LEVEL_DEPTH + WIDTH * HEIGHT + 1
                self._removeSlices(
                    WIDTH * HEIGHT + 1,
                    (self._selected.level - 1) * WIDTH * HEIGHT * LEVEL_DEPTH +
                    WIDTH * HEIGHT,
                    afterStartIndex, #blockEntityIds
                )
            elseif not self._selected.slice then
                self._selected.slice = index
                self._selectedBoundingBoxes = {}
                for blockNr = 1, WIDTH * HEIGHT do
                    table.insert(self._selectedBoundingBoxes, blockNr,
                        self._boundingBoxes.blocks[
                        WIDTH * HEIGHT * (index + (self._selected.level - 1) * LEVEL_DEPTH) + blockNr])
                end
                local firstBlockOfLevel = ((self._selected.level - 1) * LEVEL_DEPTH + 1) * WIDTH * HEIGHT + 1
                self._removeSlices(
                    firstBlockOfLevel,
                    firstBlockOfLevel + (self._selected.slice - 1) * WIDTH * HEIGHT - 1,
                    firstBlockOfLevel + (self._selected.slice) * WIDTH * HEIGHT,
                    firstBlockOfLevel + WIDTH * HEIGHT * (LEVEL_DEPTH) - 1
                )
            elseif not self._selected.block then
                self._selected.block = WIDTH * HEIGHT *
                    ((self._selected.level - 1) * LEVEL_DEPTH + (self._selected.slice - 1) + 1) + index
            end
        end
    else
        TARGET_BOUNDING_BOX = nil
    end

    if scene.IsKeyPressed(string.byte("R")) then
        if self._selected.slice then
            randomizeSlice(self._selected.level, self._selected.slice)
            local firstBlockOfLevel = ((self._selected.level - 1) * LEVEL_DEPTH + 1) * WIDTH * HEIGHT + 1
            local firstBlockOfSlice = firstBlockOfLevel + (self._selected.slice - 1) * WIDTH * HEIGHT - 1
            for blockNr = firstBlockOfSlice, firstBlockOfSlice + WIDTH * HEIGHT do
                local blockComponent = scene.GetComponent(blockEntityIds[blockNr], "block")
                self._realBlocks[blockNr] = blockComponent.material
            end
            self._removeTreasuresInSlice(self._selected.level, self._selected.slice)
        elseif self._selected.level then
            for sliceNr = 0, LEVEL_DEPTH - 1 do
                randomizeSlice(self._selected.level, sliceNr + 1)
                self._removeTreasuresInSlice(self._selected.level, sliceNr + 1)
            end

            local firstBlockOfLevel = ((self._selected.level - 1) * LEVEL_DEPTH + 1) * WIDTH * HEIGHT + 1
            for blockNr = firstBlockOfLevel, firstBlockOfLevel + WIDTH * HEIGHT * LEVEL_DEPTH do
                local blockComponent = scene.GetComponent(blockEntityIds[blockNr], "block")
                self._realBlocks[blockNr] = blockComponent.material
            end

        else
            for y = levels[1].beginningDepth + 1, #levels * LEVEL_DEPTH do
                local levelNr = math.floor((y - 1) / LEVEL_DEPTH + 1)
                local sliceNr = y - (levelNr - 1) * LEVEL_DEPTH
                randomizeSlice(levelNr, sliceNr)
                self._removeTreasuresInSlice(levelNr, sliceNr)
            end

            for blockNr = 2 * WIDTH * HEIGHT, #blockEntityIds do
                local blockComponent = scene.GetComponent(blockEntityIds[blockNr], "block")
                self._realBlocks[blockNr] = blockComponent.material
            end
        end
    end

    if self._selected.block then
        -- Change the material of the selected block
        local selectedBlockEntity = blockEntityIds[self._selected.block]
        local selectedBlockComponent = scene.GetComponent(selectedBlockEntity, "block")
        local selectedBlockMaterialId = selectedBlockComponent.material

        if scene.IsKeyPressed(string.byte("Q")) then
            selectedBlockMaterialId = selectedBlockMaterialId - 1
        elseif scene.IsKeyPressed(string.byte("E")) then
            selectedBlockMaterialId = selectedBlockMaterialId + 1
        end

        if selectedBlockMaterialId == 3 then
            selectedBlockMaterialId = 12
        elseif selectedBlockMaterialId == 13 then
            selectedBlockMaterialId = 4
        end
        local selectedBlockMaterialName = getMaterialNameFromId(selectedBlockMaterialId)
        local key = selectedBlockMaterialName and string.upper(selectedBlockMaterialName) or
            getMaterialNameFromId(BLOCK_INFO.DIRT.id)

        local selectedBlockInfo = BLOCK_INFO[key]
        self._realBlocks[self._selected.block] = selectedBlockMaterialId
        scene.SetComponent(selectedBlockEntity, "block", selectedBlockInfo.value, selectedBlockInfo.weight,
            selectedBlockComponent.position, selectedBlockComponent.size, selectedBlockMaterialId)
    end

    if scene.IsKeyPressed(string.byte("G")) and self._selected.block then
        if (self._selected.slice == 1 and self._selected.level == 1) then
            return
        end
        local blockInSlice = self._selected.block - WIDTH * HEIGHT *
            ((self._selected.level - 1) * LEVEL_DEPTH + (self._selected.slice - 1) + 1)
        local selectedX = (blockInSlice - 1) % HEIGHT + 1
        local selectedZ = math.floor((blockInSlice - 1) / WIDTH) + 1
        local reservedBlocks = (MOLE_ROOM_SIDE_LENGTH - 1) / 2
        local materialToFill = BLOCK_INFO.EMPTY.id
        if selectedX > reservedBlocks and selectedX <= WIDTH - reservedBlocks and selectedZ > reservedBlocks and
            selectedZ <= HEIGHT - reservedBlocks then

            local doesTreasureAlreadyExist = false
            for treasureIndex, treasure in ipairs(self._realTreasures[self._selected.level]) do
                if treasure.position.x == selectedX and treasure.position.y == self._selected.slice and
                    treasure.position.z == selectedZ then

                    doesTreasureAlreadyExist = true
                    materialToFill = BLOCK_INFO.DIRT.id
                    scene.RemoveComponent(treasure.id, "treasure")
                    scene.RemoveEntity(treasure.id)
                    table.remove(self._realTreasures[self._selected.level], treasureIndex)
                end
            end

            local roomStartX = selectedX - reservedBlocks
            local roomStartZ = selectedZ - reservedBlocks
            for x = roomStartX, roomStartX + MOLE_ROOM_SIDE_LENGTH - 1 do
                for z = roomStartZ, roomStartZ + MOLE_ROOM_SIDE_LENGTH - 1 do
                    local blockIndex = ((self._selected.level - 1) * LEVEL_DEPTH + self._selected.slice) * WIDTH * HEIGHT
                        + (z - 1) * WIDTH + x - 1 + 1
                    local id = blockEntityIds[blockIndex]
                    local currentBlock = scene.GetComponent(id, "block")
                    currentBlock.material = materialToFill
                    self._realBlocks[blockIndex] = currentBlock.material
                    scene.SetComponent(id, "block", currentBlock.gold, currentBlock.weight, currentBlock.position,
                        currentBlock.size, currentBlock.material)
                end
            end

            if not doesTreasureAlreadyExist then
                local treasureEntity = scene.CreateEntity()
                scene.SetComponent(treasureEntity, "treasure", self._selected.level * self._selected.level * 8000,
                    self._selected.level * self._selected.level * 100,
                    self._selected.level * self._selected.level * 100, {
                    position = {
                        x = START_X + (selectedX - 1) * BLOCK_SIZE,
                        y = START_Y - ((self._selected.level - 1) * LEVEL_DEPTH + self._selected.slice) * BLOCK_SIZE -
                            BLOCK_SIZE / 2 + 0.5,
                        z = START_Z + (selectedZ - 1) * BLOCK_SIZE
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

                table.insert(self._realTreasures[self._selected.level], {
                    id = treasureEntity,
                    position = {
                        x = selectedX,
                        y = self._selected.slice,
                        z = selectedZ
                    },
                })
            else
                materialToFill = BLOCK_INFO.EMPTY.id
                for _, treasure in ipairs(self._realTreasures[self._selected.level]) do
                    if treasure.position.y == self._selected.slice then
                        local roomStartX = treasure.position.x - reservedBlocks
                        local roomStartZ = treasure.position.z - reservedBlocks
                        for x = roomStartX, roomStartX + MOLE_ROOM_SIDE_LENGTH - 1 do
                            for z = roomStartZ, roomStartZ + MOLE_ROOM_SIDE_LENGTH - 1 do
                                local blockIndex = ((self._selected.level - 1) * LEVEL_DEPTH + self._selected.slice) *
                                    WIDTH * HEIGHT + (z - 1) * WIDTH + x - 1 + 1
                                local id = blockEntityIds[blockIndex]
                                local currentBlock = scene.GetComponent(id, "block")
                                currentBlock.material = materialToFill
                                self._realBlocks[blockIndex] = currentBlock.material
                                scene.SetComponent(id, "block", currentBlock.gold, currentBlock.weight,
                                    currentBlock.position, currentBlock.size, currentBlock.material)
                            end
                        end
                    end
                end
            end
        end
    end

    if scene.IsKeyPressed(string.byte("B")) then
        if self._selected.block then
            self._selected.block = nil
        elseif self._selected.slice then
            self._selectedBoundingBoxes = {}
            local firstBlockOfLevel = ((self._selected.level - 1) * LEVEL_DEPTH + 1) * WIDTH * HEIGHT + 1
            for blockNr = firstBlockOfLevel, firstBlockOfLevel + WIDTH * HEIGHT * LEVEL_DEPTH - 1 do
                local hiddenBlockEntity = blockEntityIds[blockNr]
                local hiddenBlockComponent = scene.GetComponent(hiddenBlockEntity, "block")
                local blockId = self._realBlocks[blockNr]
                local materialName = getMaterialNameFromId(blockId) or BLOCK_INFO.DIRT.name
                local key = string.upper(materialName)
                scene.SetComponent(hiddenBlockEntity, "block", BLOCK_INFO[key].value, BLOCK_INFO[key].weight,
                    hiddenBlockComponent.position, hiddenBlockComponent.size, blockId)
            end
            for sliceNr = 1, LEVEL_DEPTH do
                table.insert(self._selectedBoundingBoxes, sliceNr,
                    self._boundingBoxes.slices[(self._selected.level - 1) * LEVEL_DEPTH + sliceNr])
                self._showTreasuresInSlice(self._selected.level, sliceNr)
            end
            self._selected.slice = nil
        elseif self._selected.level then
            for blockNr, blockId in ipairs(self._realBlocks) do
                local hiddenBlockEntity = blockEntityIds[blockNr]
                local hiddenBlockComponent = scene.GetComponent(hiddenBlockEntity, "block")
                local materialName = getMaterialNameFromId(blockId) or BLOCK_INFO.DIRT.name
                local key = string.upper(materialName)
                scene.SetComponent(hiddenBlockEntity, "block", BLOCK_INFO[key].value, BLOCK_INFO[key].weight,
                    hiddenBlockComponent.position, hiddenBlockComponent.size, blockId)
            end
            self._selectedBoundingBoxes = self._boundingBoxes.levels

            for y = 1, #levels * LEVEL_DEPTH do
                local levelNr = math.floor((y - 1) / LEVEL_DEPTH + 1)
                local sliceNr = y - (levelNr - 1) * LEVEL_DEPTH
                self._showTreasuresInSlice(levelNr, sliceNr)
            end

            self._selected.level = nil
        end
    end

    if scene.IsKeyPressed(257) then
        -- Save to file
        local realTreasures = {}
        for levelNr, level in pairs(self._realTreasures) do
            for _, treasure in pairs(level) do
                treasure.position.y = treasure.position.y + (levelNr - 1) * LEVEL_DEPTH
                table.insert(realTreasures, treasure.position)
            end
        end

        scene.SaveTables({
            blocks = self._realBlocks,
            treasures = realTreasures
        })
    end
    if scene.IsKeyPressed(string.byte("Z")) then
        -- Exit edit mode

        local realTreasures = {}
        for levelNr, level in pairs(self._realTreasures) do
            for _, treasure in pairs(level) do
                treasure.position.y = treasure.position.y + (levelNr - 1) * LEVEL_DEPTH
                table.insert(realTreasures, treasure.position)
            end
        end

        scene.SaveTables({
            editorBlocks = self._realBlocks,
            editorTreasures = realTreasures
        })

        IS_IN_EDIT_MODE = false
    end
end

return editor
