#include "luaint/lua_state.hpp"

using namespace flat::lua;

state::state()
{
    // TODO, evaluate libraries to open
    open_libraries(sol::lib::base);

    /*
     * Texture binding
     */

    auto tex_access = new_usertype<wsdl2::texture::access>("tex_access");

    tex_access.set("static", wsdl2::texture::access::static_);
    tex_access.set("streaming", wsdl2::texture::access::streaming);
    tex_access.set("target", wsdl2::texture::access::target);

    auto tex_t = new_usertype<wsdl2::texture>("texture");

    tex_t.set("access", sol::readonly(&wsdl2::texture::pixel_access));
    tex_t.set("format", sol::readonly(&wsdl2::texture::pixel_format()));
    //tex_t.set("path", sol::readonly(&wsdl2::texture::)); TODO
    //tex_t.set("owner", &wsdl2::texture::owner);
    tex_t.set_function("lock", &wsdl2::texture::lock);
    tex_t.set_function("unlock", &wsdl2::texture::unlock);
    tex_t.set("alpha", 
        static_cast<bool (wsdl2::texture::*)(std::uint8_t)>(&wsdl2::texture::alpha));
    tex_t.set("width", sol::readonly(&wsdl2::texture::width));
    tex_t.set("height", sol::readonly(&wsdl2::texture::height));

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

    channel_type.set("emit", &channel::emit<>); // no arguments
    channel_type.set("emit", &channel::emit<sol::object>); // one object argument
    channel_type.set("emit", &channel::emit<sol::variadic_args>); // variadic arguments
    channel_type.set("broadcast", &channel::broadcast);
    // simple function connection <typename R, typename ...Args>
    channel_type.set("connect", &channel::connect<void>); // TODO, ambiguous
    channel_type.set("connect", &channel::connect<void, sol::object>);
    channel_type.set("connect", &channel::connect<void, sol::variadic_args>);

    // listener (no args) binding
    new_usertype<listener<>>("listener"); 
    // listener (sol::object) binding
    new_usertype<listener<sol::object>>("listener_obj"); 
    // listener (sol::variadic_args) binding
    new_usertype<listener<sol::variadic_args>>("listener_var"); 

    // events binding

    // namespace events
    //auto nmsp_events = set("events", );
    set("events.key.connect", &flat::state::events::connect<wsdl2::event::key>)
    set("events.key.disconnect", &flat::state::events::disconnect<wsdl2::event::key>)
    //set("disconnect_all", &flat::state::events::disconnect_all<wsdl2::event::event>)
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
