local mole = {}

function mole:OnCreate()
	self._hasTreasure = false
	self._lastStunned = os.clock()
	self._stunTime = 0.5
	self._speed = 4.5
	self._transform = {
		position = {
			x = nil,
			y = nil,
			z = nil
		},
		rotation = {
			x = 0,
			y = 0,
			z = 0
		},
		scale = {
			x = MOLE_SIZE,
			y = MOLE_SIZE,
			z = MOLE_SIZE
		}
	}

	if scene.HasComponent(self.ID, "treasure") then
		local treasureComponent = scene.GetComponent(self.ID, "treasure")
		self._roomLength = MOLE_ROOM_SIDE_LENGTH
		self._hasTreasure = true
		self._roomCenterPos = {
			x = treasureComponent.transform.position.x,
			z = treasureComponent.transform.position.z
		}
		self._groundLevelY = treasureComponent.transform.position.y - treasureComponent.transform.scale.y / 2
		self._health = treasureComponent.maxDurability * 1.5
		self._maxHealth = treasureComponent.maxDurability * 1.5
		self._damage = treasureComponent.maxDurability / 2
		self._spawnPos = {
			x = treasureComponent.transform.position.x + 3,
			y = treasureComponent.transform.position.y - treasureComponent.transform.scale.y / 2 + MOLE_SIZE / 2,
			z = treasureComponent.transform.position.z
		}
		self._transform.position = self._spawnPos
		scene.SetComponent(self.ID, "mole", self._health, self._maxHealth, self._damage, self._transform, false)
	else
		local moleComponent = scene.GetComponent(self.ID, "mole")
		self._health = moleComponent.maxHealth
		self._maxHealth = moleComponent.maxHealth
		self._damage = moleComponent.damage
		self._transform = moleComponent.transform
		scene.SetComponent(self.ID, "mole", self._health, self._maxHealth, self._damage, self._transform, false)
	end
	self._attackRadius = math.sqrt(self._transform.scale.x / 2 * self._transform.scale.x / 2 +
		self._transform.scale.z / 2 * self._transform.scale.z / 2) + 0.3

	self._delta = 0
	self._moleToCharacter = nil

	self._isPlayerInRoom = function()
		local characterComponent = scene.GetComponent(CHARACTER_ENTITY_ID, "character")
		local characterPos = characterComponent.transform.position
		local characterScale = characterComponent.transform.scale
		return characterPos.x < self._roomCenterPos.x + self._roomLength / 2 * BLOCK_SIZE and
			characterPos.x > self._roomCenterPos.x - self._roomLength / 2 * BLOCK_SIZE and
			characterPos.z < self._roomCenterPos.z + self._roomLength / 2 * BLOCK_SIZE and
			characterPos.z > self._roomCenterPos.z - self._roomLength / 2 * BLOCK_SIZE and
			characterPos.y - characterScale.y / 2 >= self._groundLevelY and
			characterPos.y + characterScale.y / 2 <= self._groundLevelY + BLOCK_SIZE
	end
	self._isCloseToPlayer = function()
		local distance = math.sqrt(
			self._moleToCharacter.x * self._moleToCharacter.x +
			self._moleToCharacter.y * self._moleToCharacter.y +
			self._moleToCharacter.z * self._moleToCharacter.z
		)

		return distance < 20
	end
	self._moveToPlayer = function()
		local distanceXZ = math.sqrt(
			self._moleToCharacter.x * self._moleToCharacter.x +
			self._moleToCharacter.z * self._moleToCharacter.z
		)
		self._distanceToPlayer = distanceXZ
		local toPlayerNormalized = {
			x = self._moleToCharacter.x / distanceXZ,
			z = self._moleToCharacter.z / distanceXZ
		}
		self._moleToCharacter = toPlayerNormalized
		scene.SetComponent(self.ID, "mole", self._health, self._maxHealth, self._damage, {
			position = {
				x = self._transform.position.x + toPlayerNormalized.x * self._speed * self._delta,
				y = self._transform.position.y,
				z = self._transform.position.z + toPlayerNormalized.z * self._speed * self._delta
			},
			rotation = { x = 0, y = 0, z = 0 },
			scale = { x = self._transform.scale.x, y = self._transform.scale.y, z = self._transform.scale.z }
		}, false)
	end
	self._moveToSpawnPos = function()
		local toSpawnPos = {
			x = self._spawnPos.x - self._transform.position.x,
			z = self._spawnPos.z - self._transform.position.z,
		}
		local toSpawnPosLength = math.sqrt(toSpawnPos.x * toSpawnPos.x + toSpawnPos.z * toSpawnPos.z)
		local toSpawnPosNormalized = {
			x = toSpawnPos.x / toSpawnPosLength,
			z = toSpawnPos.z / toSpawnPosLength
		}

		scene.SetComponent(self.ID, "mole", self._health, self._maxHealth, self._damage, {
			position = {
				x = self._transform.position.x + toSpawnPosNormalized.x * self._speed * self._delta,
				y = self._transform.position.y,
				z = self._transform.position.z + toSpawnPosNormalized.z * self._speed * self._delta
			},
			rotation = {
				x = 0,
				y = 0,
				z = 0
			},
			scale = self._transform.scale
		}, false)
	end

	self._isIdle = function()
		return math.abs(self._transform.position.x - self._spawnPos.x) < 0.1 and
			math.abs(self._transform.position.z - self._spawnPos.z) < 0.1
	end
	self._isStunned = function()
		local now = os.clock()
		if now - self._lastStunned < self._stunTime then
			return true
		end
		local moleComponent = scene.GetComponent(self.ID, "mole")
		if moleComponent.isStunned then
			self._health = moleComponent.health
			self._lastStunned = os.clock()
			scene.SetComponent(self.ID, "mole", moleComponent.health, moleComponent.maxHealth, moleComponent.damage,
				moleComponent.transform, false)
			return true
		end

		return false
	end
	self._canAttackPlayer = function()
		return self._distanceToPlayer <= self._attackRadius
	end
	self._attackPlayer = function()
		local currentHealth, maxHealth = scene.GetComponent(CHARACTER_ENTITY_ID, "health")
		currentHealth = currentHealth - self._damage
		scene.SetComponent(CHARACTER_ENTITY_ID, "health", currentHealth, maxHealth)
		self._lastStunned = os.clock()
	end
	self._update = coroutine.create(function()
		if self._hasTreasure then
			while true do -- Mole is in a room
				while self._isPlayerInRoom() do
					if not self._isStunned() then
						self._moveToPlayer()
						if self._canAttackPlayer() then
							self._attackPlayer()
						end
					end
					coroutine.yield()
				end
				if self._isIdle() then
					self._health = self._health + 0.02 * self._maxHealth * self._delta < self._maxHealth and
						self._health + 0.02 * self._maxHealth * self._delta or self._maxHealth
					scene.SetComponent(self.ID, "mole", self._health, self._maxHealth, self._damage, self._transform, false)
				else
					self._moveToSpawnPos()
				end
				coroutine.yield()
			end
		else
			while true do -- Mole is in the boss room
				if not self._isStunned() then
					self._moveToPlayer()
					if self._canAttackPlayer() then
						self._attackPlayer()
					end
				end
				coroutine.yield()
			end
		end
	end)

	print("Mole created!")
end

function mole:OnUpdate(delta)
	self._delta = delta
	self._transform = scene.GetComponent(self.ID, "mole").transform
	local characterTransform = scene.GetComponent(CHARACTER_ENTITY_ID, "character").transform
	self._moleToCharacter = {
		x = characterTransform.position.x - self._transform.position.x,
		y = characterTransform.position.y - self._transform.position.y,
		z = characterTransform.position.z - self._transform.position.z
	}
	coroutine.resume(self._update)
end

return mole
