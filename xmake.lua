set_project("Mano")
set_version("1.0.0")

set_languages("c++20")

if is_mode("debug") then
    add_defines("DEBUG")
    set_symbols("debug")
    set_optimize("none")
else 
    set_symbols("hidden")
    -- optionally we can include debug symbols in release builds, useful for profiling
    -- set_symbols("debug")
    set_optimize("fastest")
end

target("Mano")
    set_kind("binary")
    set_targetdir("bin")
    add_includedirs("src/")
    add_files("src/*.cpp")
