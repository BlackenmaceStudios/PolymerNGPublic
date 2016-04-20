// Direct3D12_RHI_InputElementDesc.cpp
//

#include "BuildRHI_Direct3D12.h"

BuildRHIInputElementDescDirect3D12UI ui_VertexElementDescriptor_local;
BuildRHIInputElementDesc *ui_VertexElementDescriptor = &ui_VertexElementDescriptor_local;

BuildRHIInputElementDescDirect3D12World world_world_VertexElementDescriptor_local;
BuildRHIInputElementDesc *world_VertexElementDescriptor = &world_world_VertexElementDescriptor_local;

BuildRHIInputElementDescDirect3D12UI::BuildRHIInputElementDescDirect3D12UI()
{
	vertElem[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	vertElem[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

BuildRHIInputElementDescDirect3D12World::BuildRHIInputElementDescDirect3D12World()
{

}