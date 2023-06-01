local stars = {}
local circlePoints = {}


function UpdateStar(star, delta)
    if (star.x > 1920) then
        star.x = 0
        star.y = math.random(1, 1080)
    else
        star.x = star.x + star.s * delta * 100
    end
end

function DrawStar(star)
    DrawRect(star.x, star.y, star.s, star.s)
end

function Main()
    math.randomseed()
    for i = 1, 100 do
        table.insert(stars,
            {
                x = math.random(1, 1920),
                y = math.random(1, 1080),
                s = math.random(1, 4)
            }
        )

    end

    for i = 1, 360 do
        table.insert(circlePoints, {
            x = math.cos(math.rad(i)) * 500 + 1920 / 2,
            y = math.sin(math.rad(i)) * 500 + 1080 / 2,
        })
    end


end

function OnUpdate(delta)
    for i, star in ipairs(stars) do
        UpdateStar(star, delta)
    end
end

function OnFrame(delta)
    for i, star in ipairs(stars) do
        DrawStar(star)
    end

    for i, p in ipairs(circlePoints) do
        DrawRect(p.x, p.y, 1, 1)
    end
end
