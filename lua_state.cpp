#include "flatlua/lua_state.hpp"
#include "flatland/flatland.hpp"

extern "C" {
#include <SDL2/SDL_events.h>
}

#include "wsdl2/video.hpp"
#include "wsdl2/event.hpp"

#include "flatlua/lua_signal.hpp"

#include <unistd.h>
#include "flatlua/resources.hpp"

template<typename ...Args>
using connect_t = std::shared_ptr<flat::core::listener<Args...>> (flat::core::channel::*)(void (*)(Args...));

using namespace flat::lua;

//resources
build::resource r_utils = LOAD_RESOURCE(scripts_utils_lua);
build::resource r_cmd = LOAD_RESOURCE(scripts_cmd_lua);
build::resource r_events = LOAD_RESOURCE(scripts_events_lua);

state::state(flat::state& engine)
{
    // TODO, evaluate libraries to open
    open_libraries(sol::lib::base);

    /*
     * Texture binding
     */

    (*this)["tex_access"] = create_table_with(
            "static", wsdl2::texture::access::static_,
            "streaming", wsdl2::texture::access::streaming,
            "target", wsdl2::texture::access::target
            );

    auto tex_t = new_usertype<wsdl2::texture>("texture");

    tex_t.set("access", sol::readonly(&wsdl2::texture::pixel_access));
    tex_t.set("format", sol::readonly(&wsdl2::texture::pixel_format));
    tex_t.set("alpha", 
        static_cast<bool (wsdl2::texture::*)(std::uint8_t)>(&wsdl2::texture::alpha));
    tex_t.set("width", sol::readonly(&wsdl2::texture::width));
    tex_t.set("height", sol::readonly(&wsdl2::texture::height));
    //tex_t.set("path", sol::readonly(&wsdl2::texture::)); TODO
    //tex_t.set("owner", &wsdl2::texture::owner);
    
    // TODO, inheritance problem
    //tex_t.set_function("lock", &wsdl2::streaming_texture::lock);
    //tex_t.set_function("unlock", &wsdl2::streaming_texture::unlock);

    /*
     * Window binding
     */

    auto window_t = new_usertype<wsdl2::window>("window");

    window_t.set("show", &wsdl2::window::show);
    window_t.set("hide", &wsdl2::window::hide);
    window_t.set("raise", &wsdl2::window::raise);
    window_t.set("opened", sol::readonly(&wsdl2::window::is_open));
    window_t.set("visible", sol::readonly(&wsdl2::window::is_visible));

    /*
     * Scene binding
     */

    auto scene_type = new_usertype<flat::scene>("scene", sol::constructors<flat::scene()>(),
            sol::base_classes, sol::bases<flat::trait::renderable, std::set<std::shared_ptr<flat::trait::renderable>>, flat::rendergroup>()
            );

    scene_type.set("title", &flat::scene::title);

    scene_type.set("load_texture", &flat::scene::load_texture);
    // most complete version of load_texture
    // TODO, tileset two constructors, grid != rectangle ??
    scene_type.set("load_tileset_from_path", static_cast<std::shared_ptr<flat::tileset> (flat::scene::*)(const std::string&, std::size_t, std::size_t, std::size_t, std::size_t)>(&flat::scene::load_tileset));
    scene_type.set("load_tileset_from_texture", static_cast<std::shared_ptr<flat::tileset> (flat::scene::*)(std::shared_ptr<wsdl2::texture>, std::size_t, std::size_t, std::size_t, std::size_t)>(&flat::scene::load_tileset));

    scene_type.set("load_sprite_from_path", static_cast<std::shared_ptr<flat::sprite> (flat::scene::*)(const std::string&, const mm::vec2<int>&, unsigned)>(&flat::scene::load_sprite));
    scene_type.set("load_sprite_from_tileset", static_cast<std::shared_ptr<flat::sprite> (flat::scene::*)(std::shared_ptr<flat::tileset>, const mm::vec2<int>&, unsigned)>(&flat::scene::load_sprite));

    using namespace flat::core;

    //Tasks, jobs, channels, signals interface

    // job binding
    auto job_type = new_usertype<job>("job", sol::constructors<job()>());

    job_type.set("delegate_task", static_cast<std::shared_ptr<task> (job::*)(task::callback, priority_t)>(&job::delegate_task));
    job_type.set("invoke_tasks", &job::invoke_tasks);
    // TODO, operator () overloading

    // task binding
    auto task_type = new_usertype<task>("task", 
            sol::constructors<task(task::callback),  // default priority
                              task(task::callback, priority_t)>());

    task_type.set("invoke", &task::operator());
    // TODO, operator () overloading
  
    /* channel binding 
    auto channel_type = new_usertype<channel>("channel",
            sol::constructors<channel(job&), channel(job&, priority_t)>(),
            //"emit", sol::overload(
            //    &channel::emit<sol::object>,
            //    &channel::emit<sol::variadic_args>),
            "connect", sol::overload(
                static_cast<connect_t<sol::object>>(&channel::connect)
                //static_cast<connect_t<sol::variadic_args>>(&channel::connect)
                ),
            "broadcast", &channel::broadcast);

        //connect_t<sol::variadic_args> a = &channel::connect;

    // listener (no args) binding
    new_usertype<listener<>>("listener"); 
    // listener (sol::object) binding
    new_usertype<listener<sol::object>>("listener_obj"); 
    // listener (sol::variadic_args) binding
    new_usertype<listener<sol::variadic_args>>("listener_var"); 
    */

    // events binding
    
    // enum string events
    (*this)["events"] = create_table_with(
            "key", event_id::key,
            "quit", event_id::quit,
            "mouse", create_table_with(
                "button", event_id::mouse_button,
                "motion", event_id::mouse_motion,
                "wheel", event_id::mouse_wheel
                ),
            "window", create_table_with(
                "shown", event_id::window_shown,
                "hidden", event_id::window_hidden,
                "exposed", event_id::window_exposed,
                "moved", event_id::window_moved,
                "resized", event_id::window_resized
                )
        );

    using namespace wsdl2::event;

   
    // bind every functionality for any events 
    new_usertype<wsdl2::event::key>("key_event", sol::no_constructor,
            "action", sol::readonly(&wsdl2::event::key::type),
            //"timestamp", sol::readonly(&key::timestamp),
            //"window_id", sol::readonly(&key::window_id),
            //"state", sol::readonly(&key::__state),
            //"repeat", sol::readonly(&key::repeat),
            "code", &wsdl2::event::key::code);

    // define a usertype for each callback argument type
    bind_event_functor<event_id::key>("key_cb");
    bind_event_functor<event_id::quit>("quit_cb");
    bind_event_functor<event_id::mouse_button>("mouse_button_cb");
    bind_event_functor<event_id::mouse_motion>("mouse_motion_cb");
    bind_event_functor<event_id::mouse_wheel>("mouse_wheel_cb");
    bind_event_functor<event_id::window_shown>("window_shown_cb");
    bind_event_functor<event_id::window_hidden>("window_hidden_cb");
    bind_event_functor<event_id::window_exposed>("window_exposed_cb");
    bind_event_functor<event_id::window_moved>("window_moved_cb");
    bind_event_functor<event_id::window_resized>("window_resized_cb");

    set_function("connect", sol::overload(
            &flat::lua::state::connect_event<wsdl2::event::key, event_id::key>,
            &flat::lua::state::connect_event<wsdl2::event::quit, event_id::quit>,
            &flat::lua::state::connect_event<mouse::button, event_id::mouse_button>,
            &flat::lua::state::connect_event<mouse::motion, event_id::mouse_motion>,
            &flat::lua::state::connect_event<mouse::wheel, event_id::mouse_wheel>,
            &flat::lua::state::connect_event<window::shown, event_id::window_shown>,
            &flat::lua::state::connect_event<window::hidden, event_id::window_hidden>,
            &flat::lua::state::connect_event<window::exposed, event_id::window_exposed>,
            &flat::lua::state::connect_event<window::moved, event_id::window_moved>,
            &flat::lua::state::connect_event<window::resized, event_id::window_resized>
           ));

    set_function("disconnect", &flat::lua::state::disconnect_event);

    // enum keys
    (*this)["keys"] = create_table_with(
                "action", create_table_with(
                    "up", key::action::up,
                    "down", key::action::down),
                "no_0", SDLK_0,
                "no_1", SDLK_1,
                "no_2", SDLK_2,
                "no_3", SDLK_3,
                "no_4", SDLK_4,
                "no_5", SDLK_5,
                "no_6", SDLK_6,
                "no_7", SDLK_7,
                "no_8", SDLK_8,
                "no_9", SDLK_9,
                "a", SDLK_a,
                "b", SDLK_b,
                "c", SDLK_c,
                "d", SDLK_d,
                "e", SDLK_e,
                "f", SDLK_f,
                "g", SDLK_g,
                "h", SDLK_h,
                "i", SDLK_i,
                "j", SDLK_j,
                "k", SDLK_k,
                "l", SDLK_l,
                "m", SDLK_m,
                "n", SDLK_n,
                "o", SDLK_o,
                "p", SDLK_p,
                "q", SDLK_q,
                "r", SDLK_r,
                "s", SDLK_s,
                "t", SDLK_t,
                "u", SDLK_u,
                "v", SDLK_v,
                "w", SDLK_w,
                "x", SDLK_x,
                "y", SDLK_y,
                "z", SDLK_z,
                "up", SDLK_UP,
                "down", SDLK_DOWN,
                "right", SDLK_RIGHT,
                "left", SDLK_LEFT,
                "f1", SDLK_F1,
                "f2", SDLK_F2,
                "f3", SDLK_F3,
                "f4", SDLK_F4,
                "f5", SDLK_F5,
                "f6", SDLK_F6,
                "f7", SDLK_F7,
                "f8", SDLK_F8,
                "f9", SDLK_F9,
                "f10", SDLK_F10,
                "f11", SDLK_F11,
                "f12", SDLK_F12,
                "f13", SDLK_F13,
                "f14", SDLK_F14,
                "f15", SDLK_F15,
                "f16", SDLK_F16,
                "f17", SDLK_F17,
                "f18", SDLK_F18,
                "f19", SDLK_F19,
                "f20", SDLK_F20,
                "f21", SDLK_F21,
                "f22", SDLK_F22,
                "f23", SDLK_F23,
                "f24", SDLK_F24,
                "esc", SDLK_ESCAPE,
                "end", SDLK_END,
                "backspace", SDLK_BACKSPACE,
                "delete", SDLK_DELETE // TODO, other keys
                );

    // namespace mouse
    (*this)["mouse"] = create_table_with(
            "button", create_table_with(
                "right", wsdl2::button::right,
                "middle", wsdl2::button::middle,
                "left", wsdl2::button::left, 
                "x1", wsdl2::button::x1,
                "x2", wsdl2::button::x2,
                "released", mouse::button::action::up,
                "pressed", mouse::button::action::down
                )
            // TODO, "wheel", 
            );

    new_usertype<flat::state>("flat", sol::no_constructor,
            "update", &flat::state::update,
            "current_scene", &flat::state::current_scene,
            "new_scene", &flat::state::new_scene,
            "push_scene", static_cast<void (flat::state::*)(const flat::scene&)>(&flat::state::push_scene),
            "pop_scene", &flat::state::pop_scene);

    // flatlan state setup
    (*this)["flat"] = &engine;

    // built-in script loading
    // TODO, give the possibility to chose what to enable
    
    auto utils_script = safe_script(r_utils.str(),
        [](lua_State*, sol::protected_function_result pfr) {
            sol::error err = pfr;
            npdebug("Could not load utils.lua: ", err.what());   
            return pfr;
        });

    auto cmd_script = safe_script(r_cmd.str(),
        [](lua_State*, sol::protected_function_result pfr) {
            sol::error err = pfr;
            npdebug("Could not load cmd.lua: ", err.what());   
            return pfr;
        });

    auto event_script = safe_script(r_events.str(),
        [](lua_State*, sol::protected_function_result pfr) {
            sol::error err = pfr;
            npdebug("Could not load events.lua: ", err.what());   
            return pfr;
        });
}

