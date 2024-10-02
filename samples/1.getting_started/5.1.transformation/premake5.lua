project "5.1.transformation"
		kind "WindowedApp"
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
			"%{IncludeDir.dx12_common}",
			"%{IncludeDir.DirectXTex}",
			"%{IncludeDir.external}"
		}

		links
		{
			"dx12_common",
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

