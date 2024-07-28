sdk_link_path = ""
sdk_include_path = ""
if (os.getenv("VULKAN_SDK") ~= nil) then
    sdk_link_path = os.getenv("VULKAN_SDK") .. "/include"
    sdk_include_path = os.getenv("VULKAN_SDK") .. "/lib/"
end

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
    cdialect "gnu17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "Engine/src/**.h", "Engine/src/**.c" }
    includedirs { "Engine/src" }
    defines { "EXPORT" }

    filter "system:windows"
        links { "gdi32" }

    filter "system:linux"
        links { "m" }

project "WaylandAdapter"
    basedir "WaylandAdapter"
    kind "SharedLib"
    language "C"
    cdialect "gnu17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "WaylandAdapter/src/**.h", "WaylandAdapter/src/**.c" }
    includedirs { "WaylandAdapter/src", "Engine/src", sdk_include_path }
    links { sdk_link_path .. "vulkan", "Engine", "wayland-client", "decor-0" }

project "VulkanRendererBackend"
    basedir "VulkanRendererBackend"
    kind "SharedLib"
    language "C"
    cdialect "gnu17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "VulkanRendererBackend/src/**.h", "VulkanRendererBackend/src/**.c" }
    includedirs { "VulkanRendererBackend/src", "Engine/src", sdk_include_path }
    links { sdk_link_path .. "vulkan", "Engine" }

project "TestBed"
    basedir "TestBed"
    kind "ConsoleApp"
    language "C"
    cdialect "gnu17"
    targetdir "bin/%{cfg.buildcfg}"
    files { "TestBed/src/**.h", "TestBed/src/**.c" }
    includedirs { "TestBed/src", "Engine/src" }
    links { "Engine" }
    dependson { "VulkanRendererBackend" }
