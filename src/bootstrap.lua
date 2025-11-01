R"lua(
local pool = {}

_G.pool = pool

local state = {}

local function _wrap_key(k)
  local scene = scenemanager:get()
  return scene.name .. '/' .. k
end

state.system = {}

setmetatable(state.system, {
  __newindex = function(t, k, v)
    cassette:set('system/' .. k, v)
  end,
  __index = function(t, k)
    return cassette:get('system/' .. k, nil)
  end,
})

setmetatable(state, {
  __newindex = function(t, k, v)
    cassette:set(_wrap_key(k), v)
  end,
  __index = function(t, k)
    return cassette:get(_wrap_key(k), nil)
  end,
})

_G.state = state

function transition(options)
  -- Destroy scenes first
  if options.destroy then
    for _, name in ipairs(options.destroy) do
      scenemanager:destroy(name)
    end
  end

  -- Register new scenes
  if options.register then
    for _, name in ipairs(options.register) do
      scenemanager:register(name)
    end
  end
end
)lua";
