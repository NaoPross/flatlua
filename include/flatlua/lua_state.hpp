#pragma once

#include <sol/sol.hpp>
#include <string.h>

#include "flatland/core/task.hpp"
#include "flatland/core/signal.hpp"

#include "flatlua/lua_signal.hpp"

//#include <typeinfo>
#include <map>

namespace flat {

    class state; // flatland state forward declaration

    namespace lua {
        class state;
    }
}

class flat::lua::state : private sol::state {

    template<event_id TYPE_ID>
    inline void bind_event_functor(const char *name)
    {
        new_usertype<event_cb<TYPE_ID>>(  
            name, sol::constructors<event_cb<TYPE_ID>(sol::function)>()
            );
    }

public:

    state(flat::state&);

    // load a file.lua
    using sol::state::load_file;

    // load a 
    using sol::state::load;
    using sol::state::script;
    using sol::state::safe_script;

    // allow to define a new usertype
    using sol::state::new_usertype;

    // allow to set/get objects in lua
    using sol::state::set;
    using sol::state::set_function;
    using sol::state::get;

private:

    // jobs collection
    std::unordered_map<std::string, flat::core::job> jobs;

    /*
     * Map each event to control memory management in lua.
     * Refer to a bind using a string is not the lighest way
     * but it's the safest one, because the lua reference can
     * be too easily lost, causing the event to be bound infinitely
     * or to provoke a leak of memory.
     */    
    std::map<std::string, event_variant> event_map; 

    // function needed to connect events safely
    template<typename T, event_id TYPE_ID>
    bool connect_event(const std::string& name, event_cb<TYPE_ID> ev)
    {
        // check for name identifier existance
        auto it = event_map.find(name);

        if (it == event_map.end())
            return false;

        // TODO, magic and not safe at all
        // A mali estremi, estremi rimedi
        event_bind<T> ptr = flat::state::get().events.connect<void, T>(
            [&ev](T ev_tab) -> void {
                ev.callback(ev_tab);
            });

        event_map.insert({name, event_variant(std::move(ptr))});

        return true;
    }

    // function needed to connect events safely
    void disconnect_event(const std::string& name)
    {
        event_map.erase(name);
    }

    // TODO, signal and channel database
};


