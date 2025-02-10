set_project("Mano")
set_version("1.0.0")

set_languages("c++20")

target("Mano")
    set_kind("binary")
    set_targetdir("bin")
    add_includedirs("src/")
    add_files("src/*.cpp")
