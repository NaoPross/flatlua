#include "lua_scene.hpp"
#include "scene.hpp"

using namespace flat;

void lua::init_scene(sol::lua_state& s)
{
    /*
     * Texture binding:
     *  - no constructors
     *  - TODO, low level options
     */
    s.new_usertype<lua::texture>("texture",
                                 "path", &lua::texture::path,
                                 "owner", &lua::texture::owner);
    /*
     * Scene binding
     */
    auto scene_type = s.new_usertype<lua::scene>("scene", sol::constructors<scene()>(),
                                                 "title", &scene::title);

    scene_type.set("load_texture", &scene::load_texture);
    scene_type.set("load_tileset", &scene::load_tileset);
    scene_type.set("load_sprite", &scene::load_sprite);
}
