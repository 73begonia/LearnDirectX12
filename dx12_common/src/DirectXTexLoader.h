#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <DirectXTex/DirectXTex.h>

class DirectXTexLoader 
{
public:
	DirectXTexLoader::DirectXTexLoader(ID3D12Device* device);
	DirectXTexLoader::~DirectXTexLoader();
	HRESULT LoadTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::wstring& filePath, ID3D12Resource** texture,
		D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
	HRESULT CreateTextureFromImage(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
		const DirectX::ScratchImage& image, ID3D12Resource** texture);

	void FlipVertically(DirectX::ScratchImage& image);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	ID3D12Resource* uploadBuffer;
};