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
)lua";
