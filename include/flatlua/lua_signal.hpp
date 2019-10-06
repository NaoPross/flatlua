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

template<typename T>
using event_bind = std::shared_ptr<flat::core::listener<T>>;

using event_variant = std::variant<
                        event_bind<wsdl2::event::key>,
                        event_bind<wsdl2::event::quit>,
                        event_bind<wsdl2::event::mouse::button>,
                        event_bind<wsdl2::event::mouse::motion>,
                        event_bind<wsdl2::event::mouse::wheel>,
                        event_bind<wsdl2::event::window::shown>,
                        event_bind<wsdl2::event::window::hidden>,
                        event_bind<wsdl2::event::window::exposed>,
                        event_bind<wsdl2::event::window::moved>,
                        event_bind<wsdl2::event::window::resized>,
                        std::shared_ptr<bool> // for nullptr
                        >;
}
