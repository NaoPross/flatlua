#pragma once

#include <sol/sol2.hpp>
#include <memory>
#include "wsdl2/video.hpp"

namespace flat {

    // scene defined in scene.hpp
    class scene;
}

namespace flat::lua {

    void init_scene(sol::lua_state&);

    struct texture {
        
        const std::string path;
        flatland::scene& owner;
        std::weak_ptr<wsdl2::texture> tex;

        texture(const std::string& _path,
                flatland::scene& _owner,
                std::shared_ptr<wsdl2::texture> ptr)
            : path(_path),
              owner(_owner),
              tex(ptr) {}
        
        std::shared_ptr<wsdl2::texture> get()
        {
            return tex.lock();
        }

        // TODO, some low level operation
    };

    class scene : public flatland::scene
    {
    public:
        using flatland::scene::scene;

        std::object lua_load_texture(const std::string& path)
        {
            auto opt = load_texture(path);
            return (opt) ? texture(path, *this, *opt) : sol::lua_nil;
        }

        // TODO, variadic args loader functions
    };

    sol::object current_scene();
    sol::object pop_scene();
    sol::object new_scene();
}
