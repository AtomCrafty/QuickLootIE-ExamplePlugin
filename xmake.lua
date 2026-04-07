-- include subprojects
includes("lib/commonlibsse-ng")

-- set project constants
set_project("QuickLoot IE Example Plugin")
set_version("0.0.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- define targets
target("QuickLoot IE Example Plugin")
    add_rules("commonlibsse-ng.plugin", {
        name = "QuickLoot IE Example Plugin",
        author = "libxse",
        description = "QuickLoot IE Example Plugin"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
