project "dx12_common"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"src/**.h",
			"src/**.cpp"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
		}

		includedirs
		{
			"src",
			"%{IncludeDir.DirectXTex}"
		}

		links
		{
			"DirectXTex"
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