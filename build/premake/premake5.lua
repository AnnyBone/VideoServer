

workspace "videosrv"
    root_dir = path.join(_MAIN_SCRIPT_DIR, "../..")
    src_dir = path.join(root_dir, "src")
    include_dir = path.join(root_dir, "include")
    foreign_dir = path.join(root_dir, "foreign")
    ffmpeg_dir = path.join(foreign_dir, "ffmpeg-20160330-git-be746ae-win64-dev")
    sdl_dir = path.join(foreign_dir, "SDL2-2.0.4")
    x264_dir = path.join(foreign_dir, "x264-snapshot-20160506-2245-stable")
    yuv_dir = path.join(foreign_dir, "libyuv-07cb92272f393f91f8d6215633fbbaa04bbf0a1f")

    configurations { "Debug", "Release" }
    platforms { "x64" }
    location(_ACTION)

    warnings "Default"

project "videosrv"
    kind "ConsoleApp"
    language "C"

    flags { "FatalWarnings", "Symbols" }
    defines { "_CRT_SECURE_NO_WARNINGS" }

    my_targetdir = path.join(root_dir, "bin/" .. _ACTION .. "/%{cfg.buildcfg}")
    targetdir(my_targetdir)
    debugdir(my_targetdir)

    links { "winmm.lib", "sdl2.lib", "libx264-148.lib", "yuv.lib" }

    includedirs {
        src_dir,
        path.join(ffmpeg_dir, "include"),
        path.join(sdl_dir, "include"),
        x264_dir,
        path.join(yuv_dir, "include")
    }
    libdirs {
        path.join(ffmpeg_dir, "lib"),
        path.join(sdl_dir, "lib/%{cfg.platform}"),
        x264_dir,
        path.join(yuv_dir, "cmake-build/Release")
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

