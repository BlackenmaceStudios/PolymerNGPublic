// PolymerNG_Image.h
//

#pragma once

//
// BuildImageHeapType
//
enum BuildImageHeapType
{
	BUILDIMAGE_CPU_HEAP_NONE = 0,
	BUILDIMAGE_ALLOW_CPUWRITES
};

//
// BuildImageOpts
//
struct BuildImageOpts
{
	BuildImageOpts();

	int width;
	int height;
	int depth;

	std::wstring			name;

	int tileNum; // If TileNum is > 0 then width and height are not used here.

	bool isHighQualityImage;

	bool allowCPUReads;
	bool isRenderTargetImage;

	byte *inputBuffer;

	BuildImageType imageType;
	BuildImageHeapType heapType;
	BuildImageFormat format;
};

/*
=====================
BuildImageOpts::BuildImageOpts
=====================
*/
POLYMER_INLINE BuildImageOpts::BuildImageOpts()
{
	tileNum = -1;
	width = -1;
	height = -1;
	depth = -1;
	imageType = IMAGETYPE_2D;
	heapType = BUILDIMAGE_CPU_HEAP_NONE;
	format = IMAGE_FORMAT_R8;
	inputBuffer = NULL;
	isHighQualityImage = false;
	isRenderTargetImage = false;
	allowCPUReads = false;
	name = L"not_set";
}

//
// BuildImage
//
class BuildImage
{
public:
	BuildImage(BuildImageOpts imageOpts);

	void					LoadInArtData();
	void					UpdateImagePost(byte *buffer);

	int						GetWidth();
	int						GetHeight();

	int						GetDepth();

	bool					IsLoaded();

	const BuildRHITexture	*GetRHITexture() { return texture; }

	const BuildImageOpts	&GetOpts() { return imageOpts; }

	bool					AllowsCPUWrites() { return (imageOpts.heapType == BUILDIMAGE_ALLOW_CPUWRITES); }
private:
	char *					ConvertARTImage();

	BuildImageOpts			imageOpts;

	const BuildRHITexture	*texture;
	std::wstring			name;
};