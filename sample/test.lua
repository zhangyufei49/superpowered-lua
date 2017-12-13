--[[
@brief dump table
--]]
local function dump(t, head)
    for k, v in pairs(t) do
        if type(v) == "table" then
            print(string.format("%s%s:", head, k))
            dump(v, head .. "    ")
        else
            print(string.format("%s%s: %s", head, k, tostring(v)))
        end
    end
end

print("----------------| dump the superpowered module table |----------------")
local sp = require("superpowered")
dump(sp, "")

print("----------------| create player |----------------")
local player = sp.player
local p = player.create(function(p, e, data)
    print(string.format("player %s receive event: [%d] and data [%s]", tostring(p), e, tostring(data)))
    if e == 0 then
        print("duration (sec): ", player.duration(p) / 1000)
    end
end)

print("----------------| create player |----------------")
player.open(p, "./test.mp3")
player.play(p)
player.set_tempo(p, 1.2, true)

--player.release(p)
