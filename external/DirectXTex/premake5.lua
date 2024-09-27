project "DirectXTex"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        includedirs
		{
            "Common",
            "DirectXTex/Shaders/Compiled"
		}

		files
		{
			"DirectXTex/**.h",
			"DirectXTex/**.cpp",
            "Common/**.h",
            "Common/**.cpp",
            "DirectXTex/Shaders/Compiled/**.inc"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
		}

		filter "system:windows"
				systemversion "latest"

		filter "configurations:Debug"
				runtime "Debug"
				symbols "on"
				buildoptions { "/permissive" }

		filter "configurations:Release"
				runtime "Release"
				optimize "on"
				buildoptions { "/permissive" }