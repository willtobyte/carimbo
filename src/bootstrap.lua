R"(
function async(fn)
  local co = coroutine.create(fn)
  local function resume()
    local ok, delay = coroutine.resume(co)
    if ok and delay and coroutine.status(co) ~= "dead" then
      timermanager:singleshot(delay, resume)
    end
  end
  resume()
end

function wait(ms)
  coroutine.yield(ms)
end
)";
