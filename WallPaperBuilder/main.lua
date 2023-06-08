local stars = {}
local circlePoints = {}
local math = math

local w, h = 0, 0
local radius = 400

function UpdateStar(star, delta)

    star.deg = (star.deg + delta * star.s) % 360
    local ddeg = math.rad(star.deg) * 10
    local rad = math.sin(ddeg * 4 * star.s + star.size) * 15 + radius
    star.x = math.cos(ddeg) * rad + w / 2
    star.y = math.sin(ddeg) * rad + h / 2

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
    
    print(DoubleInt(2, 3))
    w, h = GetWindowSize()
    print(w, h)
    w, h = math.min(1920, w), math.min(1080, h)
    math.randomseed()
    for i = 1, 5000 do
        table.insert(stars,
            {
                x = math.random(1, w),
                y = math.random(1, h),
                s = math.random(1, 4),
                size = math.random(1, 4),
                deg = math.random() * 360
            }
        )

    end

    for i = 1, 360 do
        table.insert(circlePoints, {
            x = math.cos(math.rad(i)) * 500 + w / 2,
            y = math.sin(math.rad(i)) * 500 + h / 2,
        })
    end


end

function OnUpdate(delta)
    for i, star in ipairs(stars) do
        UpdateStar(star, delta)
    end
end

function OnFrame(delta)
    SetColor(255, 255, 0, 255)
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
