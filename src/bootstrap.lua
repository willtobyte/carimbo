R"lua(
local state = {}

local function wrap(k)
  return scenemanager.current.name .. '/' .. k
end

setmetatable(state, {
  __newindex = function(t, k, v)
    cassette:set(wrap(k), v)
  end,
  __index = function(t, k)
    return cassette:get(wrap(k), nil)
  end,
})

_G.state = state
)lua";
