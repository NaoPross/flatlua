#pragma once

#include <sol/sol.hpp>
#include <string.h>

namespace flat::lua {

    class state;
}

class flat::lua::state : private sol::state {

    class lua_general_type;

    template <class T>
    class lua_type;

public:

    state();

    /*
     * Binding blocks
     */

    // Flatland scene management, bind to variable instance
    void bind_scene_man(flat::state&);

    /*
     * Script loading and running section
     */

    // run code given by string
    template <typename ...Args>
    void run_code(const std::string& code, Args... args)
    {
        // TODO, handle exception
        auto result = sol::state::load(code);

        if (result) // if valid
            result(args);
    }

    // load code given by an external file as a command
    sol::load_result load_script(const std::string& cmd, const std::string& path);

    // load code given by string as a command
    sol::load_result load_code(const std::string& cmd, const std::string& code);

    // deactivate a command
    void rm_cmd(const std::string& cmd);

    // run directly a specific script with the given arguments
    template <typename ...Args>
    bool run_script(const std::string& path, Args... args)
    {
        auto it = scripts.find(path);

        // TODO, handle exception
        if (it != scripts.end())
            auto result = (it.second)(args);

        return result.valid();
    }

    /*
     * Base commands
     */

    // run an arbitrary command
    void exec(const std::string& cmd);
    
    using sol::state::new_usertype;

private:

    // scripts loaded by files
    std::unordered_map<std::string, sol::load_result> scripts;

    // jobs
    std::unordered_map<std::string, flat::core::job> jobs;

    // channels
    std::unordered_map<std::string, flat::core::channel> channels;
};


