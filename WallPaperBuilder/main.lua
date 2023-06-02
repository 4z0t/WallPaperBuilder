local stars = {}
local circlePoints = {}


function UpdateStar(star, delta)

    star.deg = (star.deg + delta * star.s) % 360
    local ddeg = math.rad(star.deg) * 10
    local rad = math.sin(ddeg * 4 * star.s + star.size) * 15 + 400
    star.x = math.cos(ddeg) * rad + 1920 / 2
    star.y = math.sin(ddeg) * rad + 1080 / 2

    -- if (star.x > 1920) then
    --     star.x = 0
    --     star.y = math.random(1, 1080)
    -- else
    --     star.x = star.x + star.s * delta * 100
    -- end
end

function DrawStar(star)
    DrawRect(star.x, star.y, star.size, star.size)
end

function Main()
    math.randomseed()
    for i = 1, 5000 do
        table.insert(stars,
            {
                x = math.random(1, 1920),
                y = math.random(1, 1080),
                s = math.random(1, 4),
                size = math.random(1, 4),
                deg = math.random() * 360
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

    -- local cn = #circlePoints
    -- for i = 1, cn do
    --     if (i == cn) then
    --         DrawLine(circlePoints[i].x, circlePoints[i].y,
    --             circlePoints[1].x, circlePoints[1].y)
    --     else
    --         DrawLine(circlePoints[i].x, circlePoints[i].y,
    --             circlePoints[i + 1].x, circlePoints[i + 1].y)
    --     end
    -- end
end
