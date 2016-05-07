

workspace "videosrv"
    root_dir = path.join(_MAIN_SCRIPT_DIR, "../..")
    src_dir = path.join(root_dir, "src")
    include_dir = path.join(root_dir, "include")
    foreign_dir = path.join(root_dir, "foreign")
    ffmpeg_dir = path.join(foreign_dir, "ffmpeg-20160330-git-be746ae-win64-dev")
    sdl_dir = path.join(foreign_dir, "SDL2-2.0.4")
    x264_dir = path.join(foreign_dir, "x264-snapshot-20160506-2245-stable")

    configurations { "Debug", "Release" }
    platforms { "x64" }
    location(_ACTION)

    warnings "Default"

project "videosrv"
    kind "ConsoleApp"
    language "C"

    flags { "FatalWarnings", "Symbols" }

    my_targetdir = path.join(root_dir, "bin/" .. _ACTION .. "/%{cfg.buildcfg}")
    targetdir(my_targetdir)
    debugdir(my_targetdir)

    links { "winmm.lib", "sdl2.lib", "libx264-148.lib" }

    includedirs {
        src_dir,
        path.join(ffmpeg_dir, "include"),
        path.join(sdl_dir, "include"),
        x264_dir
    }
    libdirs {
        path.join(ffmpeg_dir, "lib"),
        path.join(sdl_dir, "lib/%{cfg.platform}"),
        x264_dir
    }

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "platforms:x64"
        architecture "x86_64"

    files {
        path.join(src_dir, "**.h"),
        path.join(src_dir, "**.c"),
    }

