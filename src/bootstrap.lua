R"lua(
local state = {}
local prefix = scenemanager.current .. '/'

setmetatable(state, {
  __newindex = function(t, k, v)
    cassette:set(prefix .. k, v)
  end,
  __index = function(t, k)
    return cassette:get(prefix .. k, nil)
  end,
})

_G.state = state
)lua";
