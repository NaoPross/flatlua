#pragma once

#include "flatland/flatland.hpp"
#include "flatland/core/signal.hpp"
#include "wsdl2/event.hpp"

namespace flat::lua {

// List of all bindable events
enum event_id {

    key,
    quit,
    mouse_button,
    mouse_motion,
    mouse_wheel,
    window_shown,
    window_hidden,
    window_exposed,
    window_moved,
    window_resized
};

// functor needed to bind connect for events
template<event_id TYPE_ID>
struct event_cb
{
    event_cb(sol::function cb) : callback(cb) {}

    static constexpr event_id id = TYPE_ID;
    sol::function callback;
};
              
template<typename T, event_id TYPE_ID>
std::shared_ptr<flat::core::listener<T>> connect_event(event_cb<TYPE_ID> ev)
{
    // TODO, magic and not safe at all
    // A mali estremi, estremi rimedi
    //std::shared_ptr<flat::core::listener<T>> out =
    return flat::state::get().events.connect<void, T>(
            [&ev](T ev_tab) -> void {
                ev.callback(ev_tab);
            });
}

}
