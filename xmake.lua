add_rules("mode.debug", "mode.release")

add_requires("glfw 3.3.8", "glm 0.9.9+8", "glog v0.6.0", "assimp v5.2.5", "stb 2023.01.30")
add_requires("imgui v1.89", {configs = {glfw_opengl3 = true, use_glad = true}})
add_requires("glslang", {configs = {binaryonly = true}})
add_requires("glad")


target("loo")
    set_kind("static")
    set_languages("c11", "cxx17")
    add_includedirs("include", {public = true})
    set_symbols("debug")

    add_files("src/*.cpp")
    add_packages("glfw", "glm", "glog", "imgui", "assimp", "stb", "glad", {public = true})

    -- glad
    if is_plat("macosx") then
        -- macosx offers the latest support until 4.1
        ogl_ver = "41"
    else
        ogl_ver = "46"
    end
    add_defines("OGL_" .. ogl_ver, "_USE_MATH_DEFINES", "NOMINMAX", {public = true})

    on_config(function (target)
        local ogl_ver = "4.6"
        if is_plat("macosx") then
            ogl_ver = "4.1"
        end
        cprintf("${bright green}[INFO] ${clear}glad configure using opengl %s on %s\n", ogl_ver, os.host())
    end)


    add_rules("utils.install.cmake_importfiles")
    add_rules("utils.install.pkgconfig_importfiles")
    -- add_rules("utils.symbols.export_all", {export_classes = true})
