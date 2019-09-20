require "utils" -- starts_with

-- Metatable Cmd
local Cmd = {}

function Cmd.__call(cmd, ...)
    cmd.callback(unpack(arg)) 
end

function Cmd.__tostring(cmd)
    return cmd.description
end

-- list of the commands, dictionary type
-- format: cmd_list["cmd"] = {description, callback, aliases}
local cmd_list = {}

-- execute an existing command
-- throws an error in case of non-existing command or 
-- internal error by the callback
function exec(cmd, ...)
    local command = cmd_list[cmd]

    if command == nil then
        error("No such command '" .. cmd .. "'")
    else
        -- handle internal errors
        local status, err = pcall(command(unpack(arg)))

        if status then
            -- no errors
        else
            error("Error while executing " .. cmd .. ":\n" .. err)
        end
    end
end

-- register a new command:
--
-- cmd: command name
-- desc: description of the command
-- cb: callback function 
-- ...: possible aliases
--
-- throws an error in case of already existing command cmd
function new_cmd(cmd, desc, cb, ...)

    if cmd_list[cmd] ~= nil then
        error("Already existing command: " .. cmd)
    end

    -- bind the command format table
    local command = {}

    command.description = desc
    command.callback = cb
    local al = arg
    table.remove(al) -- remove last element
    command.aliases = al

    -- set command metatable and add it to the list
    setmetatable(command, Cmd)
    cmd_list[cmd] = command
end

-- auto-complete, it returns a list of possibilities to complete the command
function auto_cmplt(text)
    local cmplt = {}

    for cmd, tab in pairs(cmd_list) do
        if starts_with(cmd, text) then
            table.insert(cmplt, cmd)
        end
        for alias in tab.aliases do
            if starts_with(alias, text) then
                table.insert(cmplt, alias)
            end
        end
    end

    return cmplt
end



--
-- basic command binding
--

new_cmd("quit",
    "Quit the game",
    function ()
        quit() -- from flatland
    end)

new_cmd("restart",
    "Restart the game",
    function ()
        restart() -- from flatland
    end)

