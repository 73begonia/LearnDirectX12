#include "DirectXTexLoader.h"
#include <algorithm>
#include "d3dx12.h"

DirectXTexLoader::DirectXTexLoader(ID3D12Device* device) : m_device(device) {}

DirectXTexLoader::~DirectXTexLoader()
{
}

HRESULT DirectXTexLoader::LoadTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::wstring& filePath, ID3D12Resource** texture,
	D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	DirectX::ScratchImage image;
	HRESULT hr = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
	if (FAILED(hr))
	{
		return hr;
	}

	FlipVertically(image);
	CreateTextureFromImage(device, commandList, image, texture);
	
	srvDesc->Format = image.GetMetadata().format;
	srvDesc->ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc->Texture2D.MipLevels = (UINT)image.GetMetadata().mipLevels;
	srvDesc->Texture2D.MostDetailedMip = 0;
	srvDesc->Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	return S_OK;
}

HRESULT DirectXTexLoader::CreateTextureFromImage(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const DirectX::ScratchImage& image, ID3D12Resource** texture)
{
	const auto& metadata = image.GetMetadata();

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	textureDesc.Format = metadata.format;
	textureDesc.Width = static_cast<UINT>(metadata.width);
	textureDesc.Height = static_cast<UINT>(metadata.height);
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(texture));
	if (FAILED(hr))
	{
		return hr;
	}

	const DirectX::Image* img = image.GetImages();

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*texture, 0, (UINT)metadata.mipLevels);
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer));
	if (FAILED(hr))
	{
		return hr;
	}

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = img->pixels;
	textureData.RowPitch = img->rowPitch;
	textureData.SlicePitch = img->slicePitch;

	UpdateSubresources(commandList, *texture, uploadBuffer, 0, 0, (UINT)metadata.mipLevels, &textureData);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(*texture,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);

	return S_OK;
}

void DirectXTexLoader::FlipVertically(DirectX::ScratchImage& image)
{
	const DirectX::Image* img = image.GetImages();
	for (size_t i = 0; i < image.GetImageCount(); ++i) {
		const DirectX::Image& srcImg = img[i];
		for (size_t y = 0; y < srcImg.height / 2; ++y) {
			uint8_t* row1 = srcImg.pixels + y * srcImg.rowPitch;
			uint8_t* row2 = srcImg.pixels + (srcImg.height - 1 - y) * srcImg.rowPitch;
			std::swap_ranges(row1, row1 + srcImg.rowPitch, row2);
		}
	}
}