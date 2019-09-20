
local event_enum = {}

event_enum[events.key] = key_cb
event_enum[events.quit] = quit_cb
event_enum[events.mouse.button] = mouse_button_cb
event_enum[events.mouse.motion] = mouse_motion_cb
event_enum[events.mouse.wheel] = mouse_wheel_cb
event_enum[events.window.shown] = window_shown_cb
event_enum[events.window.hidden] = window_hidden_cb
event_enum[events.window.exposed] = window_exposed_cb
event_enum[events.window.moved] = window_moved_cb
event_enum[events.window.resized] = window_resized_cb

-- Alias to the connect function taking
-- a different type of events.
-- In this case the binded event corresponds to
-- the event_type given in the entry
--
-- For more information see 
-- the events enumerator from the c++ binding
function connect(name, event_type, callback)

    if callback == nil or event_type == nil then
        return nil
    end

    local args = callback.Arguments

    if args == nil then
        return nil
    end

    if event_type < 0 or event_type >= table.getn(event_enum) then
        error("Event type " .. event_type .. " not supported")
    end
   
    -- use the right constructor to bind the event correctly
    return connect(name, event_enum[event_type].new(callback))
end
