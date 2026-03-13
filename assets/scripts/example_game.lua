-- =====================================================================
-- Feliss Engine — Example Lua Script
-- assets/scripts/example_game.lua
-- =====================================================================

Log.info("example_game.lua loaded")

-- ---- PlayerController class ----
PlayerController = {}
PlayerController.__index = PlayerController

function PlayerController.new(entityId)
    local self = setmetatable({}, PlayerController)
    self.entityId  = entityId
    self.speed     = 5.0
    self.jumpForce = 8.0
    self.grounded  = true
    return self
end

function PlayerController:onStart(id)
    self.entityId = id
    Log.info("PlayerController started on entity " .. tostring(id))
end

function PlayerController:onUpdate(id, dt)
    local moved = false

    -- WASD movement
    if Input.isKeyDown(Key.W) then
        Transform.translate(id,  0, 0,  self.speed * dt)
        moved = true
    end
    if Input.isKeyDown(Key.S) then
        Transform.translate(id,  0, 0, -self.speed * dt)
        moved = true
    end
    if Input.isKeyDown(Key.A) then
        Transform.translate(id, -self.speed * dt, 0, 0)
        moved = true
    end
    if Input.isKeyDown(Key.D) then
        Transform.translate(id,  self.speed * dt, 0, 0)
        moved = true
    end

    -- Sprint
    if Input.isKeyDown(Key.LeftShift) then
        Transform.translate(id, 0, 0, self.speed * dt)
    end

    -- Jump
    if Input.isKeyDown(Key.Space) and self.grounded then
        Log.debug("Jump!")
        self.grounded = false
    end
end

function PlayerController:onDestroy(id)
    Log.info("PlayerController destroyed on entity " .. tostring(id))
end

-- ---- Timer utility ----
Timer = {}
Timer.__index = Timer
function Timer.new(interval, cb)
    return setmetatable({interval=interval, elapsed=0, callback=cb}, Timer)
end
function Timer:tick(dt)
    self.elapsed = self.elapsed + dt
    if self.elapsed >= self.interval then
        self.elapsed = self.elapsed - self.interval
        if self.callback then self.callback() end
    end
end

-- ---- Scene setup ----
local function setupScene()
    -- Create player
    local player = Entity.create("Player")
    Transform.setPosition(player, 0, 1, 0)
    Transform.setScale(player, 1, 2, 1)

    -- Create a light
    local light = Entity.create("SunLight")
    Transform.setPosition(light, 10, 20, 10)

    Log.info("Scene setup complete — entities: " ..
        tostring(player) .. ", " .. tostring(light))

    -- Heartbeat timer
    local heartbeat = Timer.new(5.0, function()
        Log.debug("Heartbeat — elapsed: " .. string.format("%.1f", Time.elapsed()) .. "s")
    end)

    return player, heartbeat
end

local player, heartbeat = setupScene()
local pc = PlayerController.new(player)
pc:onStart(player)

-- ---- Global update hook (called by ScriptEngine) ----
function onUpdate(dt)
    pc:onUpdate(player, dt)
    heartbeat:tick(dt)
end
