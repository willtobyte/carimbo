R"lua(
local state = {}

local function _wrap_key(k)
  return scenemanager.current.name .. '/' .. k
end

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
