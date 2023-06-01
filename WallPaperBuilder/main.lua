local stars = {}

function UpdateStar(star)
    if (star.x > 1920) then
        star.x = 0
        star.y = math.random(1, 1080)
    else
        star.x = star.x + star.s
    end
end

function DrawStar(star)
    DrawRect(star.x, star.y, 2, 2)
end

function Main()
    math.randomseed()
    for i = 1, 100 do
        table.insert(stars, { x = math.random(1, 1920), y = math.random(1, 1080), s = math.random(1, 4) })
    end

end

function OnUpdate()
    for i, star in ipairs(stars) do
        UpdateStar(star)
    end
end

function OnFrame()
    for i, star in ipairs(stars) do
        DrawStar(star)
    end
end
