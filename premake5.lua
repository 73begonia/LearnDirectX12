workspace "LearnDirectX12"
	architecture "x86_64"
	startproject "1.1.hello_window"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["dx12_common"] = "%{wks.location}/dx12_common/src"
IncludeDir["DirectXTex"] = "%{wks.location}/external/DirectXTex"
IncludeDir["external"] = "%{wks.location}/external"

group "Dependencies"
		include "dx12_common"
		include "external/DirectXTex"
group ""

group "Samples"
	include "samples/1.getting_started/1.1.hello_window"
	include "samples/1.getting_started/2.1.hello_triangle"
	include "samples/1.getting_started/4.2.textures_combined"
	include "samples/1.getting_started/5.1.transformation"
group ""