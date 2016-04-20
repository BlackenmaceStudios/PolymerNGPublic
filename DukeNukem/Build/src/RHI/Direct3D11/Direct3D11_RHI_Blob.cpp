// Direct3D12_RHI_Blob.cpp
//

#include "BuildRHI_Direct3D11.h"

BuildRHIBlob::BuildRHIBlob()
{
	blob_buffer = NULL;
	blob_buffer_length = -1;
}

BuildRHIBlob::~BuildRHIBlob()
{
	if (blob_buffer == NULL)
		return;

	free(blob_buffer);
	blob_buffer = NULL;
}

void BuildRHIBlob::SetMemory(void *memory, size_t blob_buffer_length)
{
	this->blob_buffer_length = blob_buffer_length;
	this->blob_buffer = malloc(blob_buffer_length);
	memcpy(this->blob_buffer, memory, blob_buffer_length);
}