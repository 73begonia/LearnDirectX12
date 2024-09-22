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

group "Dependencies"
		--include "external/imgui"
group ""

include "dx12_common"
include "samples/1.getting_started/1.1.hello_window"