-- string matching

local utils = {}

function utils.starts_with(str, start)
   return str:sub(1, #start) == start
end

function utils.ends_with(str, ending)
   return ending == "" or str:sub(-#ending) == ending
end

return utils
