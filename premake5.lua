workspace "EngineC"
    configurations { "Debug", "Release" }
    toolset "gcc"

    makesettings [[
        CC=gcc
    ]]

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "system:windows"
        defines { "PLATFORM_WINDOWS" }

    filter "system:linux"
        defines { "PLATFORM_LINUX" }

    filter "system:macosx"
        defines { "PLATFORM_MACOS" }

project "Engine"
    basedir "Engine"
    kind "SharedLib"
    language "C"
    cdialect "c17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "Engine/src/**.h", "Engine/src/**.c" }
    includedirs { "Engine/src" }
    defines { "EXPORT" }

    filter "system:windows"
        links { "gdi32" }

project "TestBed"
    basedir "TestBed"
    kind "ConsoleApp"
    language "C"
    cdialect "c17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "TestBed/src/**.h", "TestBed/src/**.c" }
    includedirs { "TestBed/src", "Engine/src" }
    links { "Engine" }
