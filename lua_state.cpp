#include "luaint/lua_state.hpp"
#include "flat/state.hpp"

using namespace flat::lua;

state::state(flat::state& engine)
{
    // TODO, evaluate libraries to open
    open_libraries(sol::lib::base);

    /*
     * Texture binding
     */

    (*this)["tex_access"] = create_table_with(
            "static", wsdl2::texture::access::static_,
            "streaming", wsdl2::texture::access::stream,
            "target", wsdl2::texture::access::target,
            );

    auto tex_t = new_usertype<wsdl2::texture>("texture");

    tex_t.set("access", sol::readonly(&wsdl2::texture::pixel_access));
    tex_t.set("format", sol::readonly(&wsdl2::texture::pixel_format()));
    tex_t.set("alpha", 
        static_cast<bool (wsdl2::texture::*)(std::uint8_t)>(&wsdl2::texture::alpha));
    tex_t.set("width", sol::readonly(&wsdl2::texture::width));
    tex_t.set("height", sol::readonly(&wsdl2::texture::height));
    //tex_t.set("path", sol::readonly(&wsdl2::texture::)); TODO
    //tex_t.set("owner", &wsdl2::texture::owner);
    
    // TODO, inheritance problem
    tex_t.set_function("lock", &wsdl2::streaming_texture::lock);
    tex_t.set_function("unlock", &wsdl2::streaming_texture::unlock);

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
                                                 "title", &flat::scene::title);

    scene_type.set("load_texture", &flat::scene::load_texture);
    scene_type.set("load_tileset", &flat::scene::load_tileset);
    scene_type.set("load_sprite", &flat::scene::load_sprite);

    // bind to default state instance
    bind_scene_man(flat::state::get());

    /*
     * Tasks, jobs, channels, signals interface
     */

    using flat::core;

    // job binding
    auto job_type = new_usertype<job>("job", sol::constructor<job()>);

    job_type.set("delegate_task", &job::delegate_task);
    job_type.set("invoke_tasks", &job::invoke_tasks);
    // TODO, operator () overloading

    // task binding
    auto task_type = new_usertype<task>("task", 
            sol::constructor<task(task::callback)>, // default priority
            sol::constructor<task(task::callback, priority_t)>);

    task_type.set("invoke", task::operator());
    // TODO, operator () overloading
   
    // channel binding 
    auto channel_type = new_usertype<channel>("channel",
            sol::constructor<channel(job&)>, // default priority
            sol::constructor<channel(job&, priority_t)>);

    channel_type.set("emit", sol::overload(
                &channel::emit<>,
                &channel::emit<sol::object>,
                &channel::emit<sol::variadic_args>));

    channel_type.set("broadcast", &channel::broadcast);

    // simple function connection <typename R, typename ...Args>
    // TODO, possible ambiguity
    channel_type.set("connect", sol::overload(
                &channel::connect<void>,
                &channel::connect<void, sol::object>,
                &channel::connect<void, sol::variadic_args>));

    // listener (no args) binding
    new_usertype<listener<>>("listener"); 
    // listener (sol::object) binding
    new_usertype<listener<sol::object>>("listener_obj"); 
    // listener (sol::variadic_args) binding
    new_usertype<listener<sol::variadic_args>>("listener_var"); 

    // events binding
    
    // enum string events
    (*this)["events"] = create_table_with(
            "key", "connect_key",
            "quit", "connect_quit",
            "mouse", create_table_with(
                "button", "connect_mouse_button",
                "motion", "connect_mouse_motion",
                "wheel", "connect_mouse_wheel",
                ),
            "window", create_table_with(
                "shown", "connect_window_shown",
                "hidden", "connect_window_hidden",
                "exposed", "connect_window_exposed",
                "moved", "connect_window_moved",
                "resized", "connect_window_resized"
                )
        );

    channel_type.set((*this)["events.key"], &channel::connect<void, wsdl2::event::key>);
    channel_type.set((*this)["events.quit"], &channel::connect<void, wsdl2::event::quit>);
    channel_type.set((*this)["events.mouse.button"], &channel::connect<void, wsdl2::event::mouse::button>);
    channel_type.set((*this)["events.mouse.motion"], &channel::connect<void, wsdl2::event::mouse::motion>);
    channel_type.set((*this)["events.mouse.wheel"], &channel::connect<void, wsdl2::event::mouse::wheel>);
    channel_type.set((*this)["events.window.shown"], &channel::connect<void, wsdl2::event::window::shown>);
    channel_type.set((*this)["events.window.hidden"], &channel::connect<void, wsdl2::event::window::hidden>);
    channel_type.set((*this)["events.window.exposed"], &channel::connect<void, wsdl2::event::window::exposed>);
    channel_type.set((*this)["events.window.moved"], &channel::connect<void, wsdl2::event::window::moved>);
    channel_type.set((*this)["events.window.resized"], &channel::connect<void, wsdl2::event::window::resized);

    using namespace wsdl2::event;

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
            "button" = create_table_with(
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

    // flatlan state setup
    (*this)["flat"] = create_table_with(
            "events", &engine.events,
            "update", &engine.update);
}


void state::bind_scene_man(flat::state& state)
{
    set_function("current_scene", &flat::state::current_scene, flat::state::get());
    set_function("new_scene", &flat::state::new_scene, flat::state::get());
    set_function("push_scene", &flat::state::push_scene, flat::state::get());
    set_function("pop_scene", &flat::state::pop_scene, flat::state::get());
}

sol::load_result state::load_script(const std::string& cmd, const std::string& path)
{
    auto result = this->load_file(path);

    if (result)
        scripts.insert_or_assign(std::move(result));

    return result;
}


sol::load_result state::load_code(const std::string& cmd, const std::string& code)
{
    auto result = this->load(code);

    if (result)
        scripts.insert_or_assign(std::move(result));

    return result;
}

void state::rm_cmd(const std::string& cmd)
{
    auto it = scripts.find(cmd);

    if (it != scripts.end())
        scripts.erase(it);
}

void exec(const std::string& cmd)
{
    channels["cmd"].emit(cmd);
}