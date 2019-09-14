
function connect(event_type, callback)
    local args = callback.Arguments
    if args == nil or table.getn(args) then
        return nil
    end

    local conn_fct = loadstring("channel." .. event_type)
    if conn_fct ~= nil then
        return flat.events:conn_fct(callback)
    else
        return nil
    end
end
