

workspace "librecorder"
    root_dir = path.join(_MAIN_SCRIPT_DIR, "../..")
    src_dir = path.join(root_dir, "src")
    include_dir = path.join(root_dir, "include")
    foreign_dir = path.join(root_dir, "foreign")
    ffmpeg_dir = path.join(foreign_dir, "ffmpeg-20160330-git-be746ae-win64-dev")

    configurations { "Debug", "Release" }
    platforms { "x64" }
    location(_ACTION)

    warnings "Default"
    flags { "FatalWarnings", "Symbols" }

project "librecorder"
    kind "ConsoleApp"
    language "C"

    includedirs(include_dir)

    my_targetdir = path.join(root_dir, "bin/" .. _ACTION .. "/%{cfg.buildcfg}")
    targetdir(my_targetdir)
    debugdir(my_targetdir)

    filter "configurations:Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "platforms:x64"
        architecture "x86_64"

    files {
        path.join(src_dir, "**.h"),
        path.join(src_dir, "**.c"),
    }
