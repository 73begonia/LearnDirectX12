#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 Pos;
};

class hello_triangle : public D3DApp
{
public:
	hello_triangle(HINSTANCE hInstance);
	hello_triangle(const hello_triangle& rhs) = delete;
	hello_triangle& operator=(const hello_triangle& rhs) = delete;
	~hello_triangle();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildTriangleGeometry();
	void BuildPSO();

private:

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unique_ptr<MeshGeometry> mTriangleGeo = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		hello_triangle theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

hello_triangle::hello_triangle(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

hello_triangle::~hello_triangle()
{
}

bool hello_triangle::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildTriangleGeometry();
	BuildPSO();

	// Execute the initialization commands
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete
	FlushCommandQueue();

	return true;
}

void hello_triangle::OnResize()
{
	D3DApp::OnResize();
}

void hello_triangle::Update(const GameTimer& gt)
{
}

void hello_triangle::Draw(const GameTimer& gt)
{
	// Reuse the memory associated with command recording
	// We can only reset when the associated command lists have finished execution on the GPU
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	float clearColor[] = { 0.2f, 0.3f, 0.3f, 1.0f };
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->IASetVertexBuffers(0, 1, &mTriangleGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mTriangleGeo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->DrawIndexedInstanced(
		mTriangleGeo->DrawArgs["triangle"].IndexCount,
		1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete. This waiting is inefficient and is
	// done for simplicity. Later we will show how to organize our rendering code
	// so we do not have to wait per frame
	FlushCommandQueue();
}

void hello_triangle::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	slotRootParameter[0].InitAsConstants(0, 0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void hello_triangle::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	mvsByteCode = d3dUtil::CompileShader(L"shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void hello_triangle::BuildTriangleGeometry()
{
	std::array<Vertex, 4> vertices =
	{
		Vertex(XMFLOAT3(-0.5f, -0.5f, 0.0f)),
		Vertex(XMFLOAT3(-0.5f, +0.5f, 0.0f)),
		Vertex(XMFLOAT3(+0.5f, +0.5f, 0.0f)),
		Vertex(XMFLOAT3(+0.5f, -0.5f, 0.0f))
	};

	std::array<std::uint16_t, 6> indices =
	{
		0, 1, 2,
		0, 2, 3
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mTriangleGeo = std::make_unique<MeshGeometry>();
	mTriangleGeo->Name = "triangleGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mTriangleGeo->VertexBufferCPU));
	CopyMemory(mTriangleGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mTriangleGeo->IndexBufferCPU));
	CopyMemory(mTriangleGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mTriangleGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mTriangleGeo->VertexBufferUploader);

	mTriangleGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mTriangleGeo->IndexBufferUploader);

	mTriangleGeo->VertexByteStride = sizeof(Vertex);
	mTriangleGeo->VertexBufferByteSize = vbByteSize;
	mTriangleGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mTriangleGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mTriangleGeo->DrawArgs["triangle"] = submesh;
}

void hello_triangle::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaState - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}