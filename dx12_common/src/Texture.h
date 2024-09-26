#pragma once
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;
class Texture
{
public:
	Texture::Texture() {}
	Texture::Texture(ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc)
		: resource(resource), srv_desc(srv_desc) {}
	Texture::~Texture() {}

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
	ComPtr<ID3D12Resource> resource;
};