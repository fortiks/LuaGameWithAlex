CHARACTER_ENTITY_ID = scene.CreateEntity()

IS_IN_EDIT_MODE = false
HAS_GAME_BEEN_LOADED = false

scene.LoadTables({
    "blocks",
    "treasures",
    "character",
    "editorBlocks",
    "editorTreasures",
    "tempCharacter"
})

local blocksToLoad = nil
local treasuresToLoad = nil

if saves.editorBlocks and saves.tempCharacter and saves.editorTreasures then
    blocksToLoad = saves.editorBlocks
    treasuresToLoad = saves.editorTreasures
elseif saves.blocks and saves.treasures then
    blocksToLoad = saves.blocks
    treasuresToLoad = saves.treasures
else
    return false
end


for blockNr, blockMaterial in ipairs(blocksToLoad) do
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

for _, treasurePosition in pairs(treasuresToLoad) do
    local treasureEntity = scene.CreateEntity()
    local levelNr = math.floor((treasurePosition.y - 1) / LEVEL_DEPTH) + 1
    scene.SetComponent(treasureEntity, "treasure", levelNr * levelNr * 8000, levelNr * levelNr * 100,
        levelNr * levelNr * 100, {
        position = {
            x = START_X + treasurePosition.x * BLOCK_SIZE,
            y = START_Y - treasurePosition.y * BLOCK_SIZE - BLOCK_SIZE / 2 + TREASURE_SIZE / 2,
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

    scene.SetComponent(treasureEntity, "behaviour", LUA_FILES_FOLDER .. "Mole.lua")
end

HAS_GAME_BEEN_LOADED = true

scene.SetComponent(CHARACTER_ENTITY_ID, "behaviour", LUA_FILES_FOLDER .. "Character.lua")
scene.SetComponent(scene.CreateEntity(), "behaviour", LUA_FILES_FOLDER .. "BossRoom.lua")

scene.DeleteFiles({
    "editorBlocks",
    "editorTreasures",
    "tempCharacter"
})

return true
