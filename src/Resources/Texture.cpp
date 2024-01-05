#include <format>
#include <wincodec.h>
#include <iostream>

#include "Resources/Texture.h"
#include "IO/BinaryWriter.h"

// HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)
#define HRESULT_E_ARITHMETIC_OVERFLOW static_cast<HRESULT>(0x80070216L)

// HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
#define HRESULT_E_NOT_SUPPORTED static_cast<HRESULT>(0x80070032L)

// HRESULT_FROM_WIN32(ERROR_HANDLE_EOF)
#define HRESULT_E_HANDLE_EOF static_cast<HRESULT>(0x80070026L)

// HRESULT_FROM_WIN32(ERROR_INVALID_DATA)
#define HRESULT_E_INVALID_DATA static_cast<HRESULT>(0x8007000DL)

// HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE)
#define HRESULT_E_FILE_TOO_LARGE static_cast<HRESULT>(0x800700DFL)

// HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE)
#define HRESULT_E_CANNOT_MAKE static_cast<HRESULT>(0x80070052L)

// HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)
#ifndef E_NOT_SUFFICIENT_BUFFER
#define E_NOT_SUFFICIENT_BUFFER static_cast<HRESULT>(0x8007007AL)
#endif

Texture::~Texture()
{
    if (data)
    {
        delete[] data;
    }
}

void Texture::Deserialize(const std::string& texPath)
{
    BinaryReader binaryReader = BinaryReader(texPath);

    Deserialize(binaryReader);
}

void Texture::Deserialize(void* data, const unsigned int dataSize)
{
    BinaryReader binaryReader = BinaryReader(data, dataSize);

    Deserialize(binaryReader);
}

void Texture::Deserialize(BinaryReader& binaryReader)
{
    textureMapHeader = binaryReader.Read<ZTextureMap::STextureMapHeader>();
    data = binaryReader.Read<unsigned char>(binaryReader.GetSize() - binaryReader.GetPosition());
    texMetadata = GenerateTexMetadata();

    scratchImage.Initialize(texMetadata);

    memcpy(scratchImage.GetPixels(), data, scratchImage.GetPixelsSize());

    if (static_cast<ERenderFormat>(textureMapHeader.nFormat) == ERenderFormat::RENDER_FORMAT_R8G8_UNORM ||
        static_cast<ERenderFormat>(textureMapHeader.nFormat) == ERenderFormat::RENDER_FORMAT_BC5_UNORM)
    {
        const HRESULT result = DirectX::Convert(*scratchImage.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM, static_cast<DirectX::TEX_FILTER_FLAGS>(0), 0, convertedScratchImage);

        if (FAILED(result))
        {
            std::cout << "Failed to convert texture." << std::endl;

            return;
        }

        for (size_t i = 2; i < convertedScratchImage.GetPixelsSize(); i += 4)
        {
            convertedScratchImage.GetPixels()[i] = 0xFF;
        }
    }
}

ZTextureMap::SMipLevel Texture::GetMipLevel(unsigned int level)
{
    ZTextureMap::SMipLevel mipLevel;
    SRenderFormatDesc pDesc;
    ERenderFormat eFormat = static_cast<ERenderFormat>(textureMapHeader.nFormat);

    mipLevel.nWidth = textureMapHeader.nWidth;
    mipLevel.nHeight = textureMapHeader.nHeight;

    GetRenderFormatDescription(&pDesc, eFormat);

    mipLevel.nSizeInBytes = pDesc.nBytesPerBlock *
        ((pDesc.nBlockWidth + mipLevel.nWidth - 1) /
            pDesc.nBlockWidth *
            ((pDesc.nBlockHeight + mipLevel.nHeight - 1) /
                pDesc.nBlockHeight));

    unsigned int offset = 0;

    if (level)
    {
        int i = 0;

        do
        {
            unsigned int width = mipLevel.nWidth + 1;
            unsigned int height = mipLevel.nHeight + 1;

            offset += mipLevel.nSizeInBytes;
            width >>= 1;
            height >>= 1;
            mipLevel.nWidth = width;
            mipLevel.nHeight = height;

            GetRenderFormatDescription(&pDesc, eFormat);

            i = level-- == 1;

            mipLevel.nSizeInBytes = pDesc.nBytesPerBlock *
                ((pDesc.nBlockWidth + width - 1) /
                    pDesc.nBlockWidth *
                    ((pDesc.nBlockHeight + height - 1) /
                        pDesc.nBlockHeight));
        }
        while (!i);
    }

    mipLevel.pData = &data[offset];

    return mipLevel;
}

DXGI_FORMAT Texture::GetDXGIFormat(const ERenderFormat renderFormat)
{
    DXGI_FORMAT result;

    switch (renderFormat)
    {
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_TYPELESS:
            result = DXGI_FORMAT_R32G32B32A32_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_FLOAT:
            result = DXGI_FORMAT_R32G32B32A32_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_UINT:
            result = DXGI_FORMAT_R32G32B32A32_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_SINT:
            result = DXGI_FORMAT_R32G32B32A32_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32_TYPELESS:
            result = DXGI_FORMAT_R32G32B32_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32_FLOAT:
            result = DXGI_FORMAT_R32G32B32_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32_UINT:
            result = DXGI_FORMAT_R32G32B32_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32_SINT:
            result = DXGI_FORMAT_R32G32B32_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_TYPELESS:
            result = DXGI_FORMAT_R16G16B16A16_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_FLOAT:
            result = DXGI_FORMAT_R16G16B16A16_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_UNORM:
            result = DXGI_FORMAT_R16G16B16A16_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_UINT:
            result = DXGI_FORMAT_R16G16B16A16_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_SNORM:
            result = DXGI_FORMAT_R16G16B16A16_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_SINT:
            result = DXGI_FORMAT_R16G16B16A16_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32_TYPELESS:
            result = DXGI_FORMAT_R32G32_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32_FLOAT:
            result = DXGI_FORMAT_R32G32_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32_UINT:
            result = DXGI_FORMAT_R32G32_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G32_SINT:
            result = DXGI_FORMAT_R32G32_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32G8X24_TYPELESS:
            result = DXGI_FORMAT_R32G8X24_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_D32_FLOAT_S8X24_UINT:
            result = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            result = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_X32_TYPELESS_G8X24_UINT:
            result = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_TYPELESS:
            result = DXGI_FORMAT_R10G10B10A2_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_UNORM:
            result = DXGI_FORMAT_R10G10B10A2_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_UINT:
            result = DXGI_FORMAT_R10G10B10A2_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R11G11B10_FLOAT:
            result = DXGI_FORMAT_R11G11B10_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_TYPELESS:
            result = DXGI_FORMAT_R8G8B8A8_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM:
            result = DXGI_FORMAT_R8G8B8A8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM_SRGB:
            result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UINT:
            result = DXGI_FORMAT_R8G8B8A8_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_SNORM:
            result = DXGI_FORMAT_R8G8B8A8_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_SINT:
            result = DXGI_FORMAT_R8G8B8A8_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_TYPELESS:
            result = DXGI_FORMAT_R16G16_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_FLOAT:
            result = DXGI_FORMAT_R16G16_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_UNORM:
            result = DXGI_FORMAT_R16G16_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_UINT:
            result = DXGI_FORMAT_R16G16_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_SNORM:
            result = DXGI_FORMAT_R16G16_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16G16_SINT:
            result = DXGI_FORMAT_R16G16_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32_TYPELESS:
            result = DXGI_FORMAT_R32_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_D32_FLOAT:
            result = DXGI_FORMAT_D32_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32_FLOAT:
            result = DXGI_FORMAT_R32_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32_UINT:
        case ERenderFormat::RENDER_FORMAT_INDEX_32:
            result = DXGI_FORMAT_R32_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R32_SINT:
            result = DXGI_FORMAT_R32_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R24G8_TYPELESS:
            result = DXGI_FORMAT_R24G8_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_D24_UNORM_S8_UINT:
            result = DXGI_FORMAT_D24_UNORM_S8_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R24_UNORM_X8_TYPELESS:
            result = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_X24_TYPELESS_G8_UINT:
            result = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R9G9B9E5_SHAREDEXP:
            result = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_B8G8_UNORM:
            result = DXGI_FORMAT_R8G8_B8G8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_G8R8_G8B8_UNORM:
            result = DXGI_FORMAT_G8R8_G8B8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_TYPELESS:
            result = DXGI_FORMAT_R8G8_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_UNORM:
            result = DXGI_FORMAT_R8G8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_UINT:
            result = DXGI_FORMAT_R8G8_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_SNORM:
            result = DXGI_FORMAT_R8G8_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_SINT:
            result = DXGI_FORMAT_R8G8_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_TYPELESS:
            result = DXGI_FORMAT_R16_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_FLOAT:
            result = DXGI_FORMAT_R16_FLOAT;

            break;
        case ERenderFormat::RENDER_FORMAT_D16_UNORM:
            result = DXGI_FORMAT_D16_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_UNORM:
            result = DXGI_FORMAT_R16_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_UINT:
        case ERenderFormat::RENDER_FORMAT_INDEX_16:
            result = DXGI_FORMAT_R16_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_SNORM:
            result = DXGI_FORMAT_R16_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R16_SINT:
            result = DXGI_FORMAT_R16_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_B5G6R5_UNORM:
            result = DXGI_FORMAT_B5G6R5_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_B5G5R5A1_UNORM:
            result = DXGI_FORMAT_B5G5R5A1_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8_TYPELESS:
            result = DXGI_FORMAT_R8_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_R8_UNORM:
            result = DXGI_FORMAT_R8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8_UINT:
            result = DXGI_FORMAT_R8_UINT;

            break;
        case ERenderFormat::RENDER_FORMAT_R8_SNORM:
            result = DXGI_FORMAT_R8_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R8_SINT:
            result = DXGI_FORMAT_R8_SINT;

            break;
        case ERenderFormat::RENDER_FORMAT_A8_UNORM:
            result = DXGI_FORMAT_A8_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_R1_UNORM:
            result = DXGI_FORMAT_R1_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC1_TYPELESS:
            result = DXGI_FORMAT_BC1_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_BC1_UNORM:
            result = DXGI_FORMAT_BC1_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC1_UNORM_SRGB:
            result = DXGI_FORMAT_BC1_UNORM_SRGB;

            break;
        case ERenderFormat::RENDER_FORMAT_BC2_TYPELESS:
            result = DXGI_FORMAT_BC2_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_BC2_UNORM:
            result = DXGI_FORMAT_BC2_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC2_UNORM_SRGB:
            result = DXGI_FORMAT_BC2_UNORM_SRGB;

            break;
        case ERenderFormat::RENDER_FORMAT_BC3_TYPELESS:
            result = DXGI_FORMAT_BC3_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_BC3_UNORM:
            result = DXGI_FORMAT_BC3_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC3_UNORM_SRGB:
            result = DXGI_FORMAT_BC3_UNORM_SRGB;

            break;
        case ERenderFormat::RENDER_FORMAT_BC4_TYPELESS:
            result = DXGI_FORMAT_BC4_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_BC4_UNORM:
            result = DXGI_FORMAT_BC4_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC4_SNORM:
            result = DXGI_FORMAT_BC4_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC5_TYPELESS:
            result = DXGI_FORMAT_BC5_TYPELESS;

            break;
        case ERenderFormat::RENDER_FORMAT_BC5_UNORM:
            result = DXGI_FORMAT_BC5_UNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_BC5_SNORM:
            result = DXGI_FORMAT_BC5_SNORM;

            break;
        case ERenderFormat::RENDER_FORMAT_LE_X2R10G10B10_UNORM:
            std::cout << "LE_X2R10G10B10 texture type isn't supported!" << std::endl;
        case ERenderFormat::RENDER_FORMAT_LE_X8R8G8B8_UNORM:
            std::cout << "LE_X8R8G8B8 texture type isn't supported!" << std::endl;
        case ERenderFormat::RENDER_FORMAT_X16Y16Z16_SNORM:
            std::cout << "X16Y16Z16 texture type isn't supported!" << std::endl;
    }

    return result;
}

ERenderFormat Texture::GetRenderFormat(const DXGI_FORMAT dxgiFormat)
{
    ERenderFormat result;

    switch (dxgiFormat)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32A32_TYPELESS;

            break;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32A32_FLOAT;

            break;
        case DXGI_FORMAT_R32G32B32A32_UINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32A32_UINT;

            break;
        case DXGI_FORMAT_R32G32B32A32_SINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32A32_SINT;

            break;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32_TYPELESS;

            break;
        case DXGI_FORMAT_R32G32B32_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32_FLOAT;

            break;
        case DXGI_FORMAT_R32G32B32_UINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32_UINT;

            break;
        case DXGI_FORMAT_R32G32B32_SINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32B32_SINT;

            break;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_TYPELESS;

            break;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_FLOAT;

            break;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_UNORM;

            break;
        case DXGI_FORMAT_R16G16B16A16_UINT:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_UINT;

            break;
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_SNORM;

            break;
        case DXGI_FORMAT_R16G16B16A16_SINT:
            result = ERenderFormat::RENDER_FORMAT_R16G16B16A16_SINT;

            break;
        case DXGI_FORMAT_R32G32_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32G32_TYPELESS;

            break;
        case DXGI_FORMAT_R32G32_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R32G32_FLOAT;

            break;
        case DXGI_FORMAT_R32G32_UINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32_UINT;

            break;
        case DXGI_FORMAT_R32G32_SINT:
            result = ERenderFormat::RENDER_FORMAT_R32G32_SINT;

            break;
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32G8X24_TYPELESS;

            break;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            result = ERenderFormat::RENDER_FORMAT_D32_FLOAT_S8X24_UINT;

            break;
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32_FLOAT_X8X24_TYPELESS;

            break;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            result = ERenderFormat::RENDER_FORMAT_X32_TYPELESS_G8X24_UINT;

            break;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R10G10B10A2_TYPELESS;

            break;
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R10G10B10A2_UNORM;

            break;
        case DXGI_FORMAT_R10G10B10A2_UINT:
            result = ERenderFormat::RENDER_FORMAT_R10G10B10A2_UINT;

            break;
        case DXGI_FORMAT_R11G11B10_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R11G11B10_FLOAT;

            break;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_TYPELESS;

            break;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM;

            break;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM_SRGB;

            break;
        case DXGI_FORMAT_R8G8B8A8_UINT:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_UINT;

            break;
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_SNORM;

            break;
        case DXGI_FORMAT_R8G8B8A8_SINT:
            result = ERenderFormat::RENDER_FORMAT_R8G8B8A8_SINT;

            break;
        case DXGI_FORMAT_R16G16_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R16G16_TYPELESS;

            break;
        case DXGI_FORMAT_R16G16_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R16G16_FLOAT;

            break;
        case DXGI_FORMAT_R16G16_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R16G16_UNORM;

            break;
        case DXGI_FORMAT_R16G16_UINT:
            result = ERenderFormat::RENDER_FORMAT_R16G16_UINT;

            break;
        case DXGI_FORMAT_R16G16_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R16G16_SNORM;

            break;
        case DXGI_FORMAT_R16G16_SINT:
            result = ERenderFormat::RENDER_FORMAT_R16G16_SINT;

            break;
        case DXGI_FORMAT_R32_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R32_TYPELESS;

            break;
        case DXGI_FORMAT_D32_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_D32_FLOAT;

            break;
        case DXGI_FORMAT_R32_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R32_FLOAT;

            break;
        case DXGI_FORMAT_R32_UINT:
            result = ERenderFormat::RENDER_FORMAT_R32_UINT;

            break;
        case DXGI_FORMAT_R32_SINT:
            result = ERenderFormat::RENDER_FORMAT_R32_SINT;

            break;
        case DXGI_FORMAT_R24G8_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R24G8_TYPELESS;

            break;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            result = ERenderFormat::RENDER_FORMAT_D24_UNORM_S8_UINT;

            break;
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R24_UNORM_X8_TYPELESS;

            break;
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            result = ERenderFormat::RENDER_FORMAT_X24_TYPELESS_G8_UINT;

            break;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            result = ERenderFormat::RENDER_FORMAT_R9G9B9E5_SHAREDEXP;

            break;
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R8G8_B8G8_UNORM;

            break;
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_G8R8_G8B8_UNORM;

            break;
        case DXGI_FORMAT_R8G8_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R8G8_TYPELESS;

            break;
        case DXGI_FORMAT_R8G8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R8G8_UNORM;

            break;
        case DXGI_FORMAT_R8G8_UINT:
            result = ERenderFormat::RENDER_FORMAT_R8G8_UINT;

            break;
        case DXGI_FORMAT_R8G8_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R8G8_SNORM;

            break;
        case DXGI_FORMAT_R8G8_SINT:
            result = ERenderFormat::RENDER_FORMAT_R8G8_SINT;

            break;
        case DXGI_FORMAT_R16_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R16_TYPELESS;

            break;
        case DXGI_FORMAT_R16_FLOAT:
            result = ERenderFormat::RENDER_FORMAT_R16_FLOAT;

            break;
        case DXGI_FORMAT_D16_UNORM:
            result = ERenderFormat::RENDER_FORMAT_D16_UNORM;

            break;
        case DXGI_FORMAT_R16_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R16_UNORM;

            break;
        case DXGI_FORMAT_R16_UINT:
            result = ERenderFormat::RENDER_FORMAT_R16_UINT;

            break;
        case DXGI_FORMAT_R16_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R16_SNORM;

            break;
        case DXGI_FORMAT_R16_SINT:
            result = ERenderFormat::RENDER_FORMAT_R16_SINT;

            break;
        case DXGI_FORMAT_B5G6R5_UNORM:
            result = ERenderFormat::RENDER_FORMAT_B5G6R5_UNORM;

            break;
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            result = ERenderFormat::RENDER_FORMAT_B5G5R5A1_UNORM;

            break;
        case DXGI_FORMAT_R8_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_R8_TYPELESS;

            break;
        case DXGI_FORMAT_R8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R8_UNORM;

            break;
        case DXGI_FORMAT_R8_UINT:
            result = ERenderFormat::RENDER_FORMAT_R8_UINT;

            break;
        case DXGI_FORMAT_R8_SNORM:
            result = ERenderFormat::RENDER_FORMAT_R8_SNORM;

            break;
        case DXGI_FORMAT_R8_SINT:
            result = ERenderFormat::RENDER_FORMAT_R8_SINT;

            break;
        case DXGI_FORMAT_A8_UNORM:
            result = ERenderFormat::RENDER_FORMAT_A8_UNORM;

            break;
        case DXGI_FORMAT_R1_UNORM:
            result = ERenderFormat::RENDER_FORMAT_R1_UNORM;

            break;
        case DXGI_FORMAT_BC1_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_BC1_TYPELESS;

            break;
        case DXGI_FORMAT_BC1_UNORM:
            result = ERenderFormat::RENDER_FORMAT_BC1_UNORM;

            break;
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            result = ERenderFormat::RENDER_FORMAT_BC1_UNORM_SRGB;

            break;
        case DXGI_FORMAT_BC2_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_BC2_TYPELESS;

            break;
        case DXGI_FORMAT_BC2_UNORM:
            result = ERenderFormat::RENDER_FORMAT_BC2_UNORM;

            break;
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            result = ERenderFormat::RENDER_FORMAT_BC2_UNORM_SRGB;

            break;
        case DXGI_FORMAT_BC3_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_BC3_TYPELESS;

            break;
        case DXGI_FORMAT_BC3_UNORM:
            result = ERenderFormat::RENDER_FORMAT_BC3_UNORM;

            break;
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            result = ERenderFormat::RENDER_FORMAT_BC3_UNORM_SRGB;

            break;
        case DXGI_FORMAT_BC4_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_BC4_TYPELESS;

            break;
        case DXGI_FORMAT_BC4_UNORM:
            result = ERenderFormat::RENDER_FORMAT_BC4_UNORM;

            break;
        case DXGI_FORMAT_BC4_SNORM:
            result = ERenderFormat::RENDER_FORMAT_BC4_SNORM;

            break;
        case DXGI_FORMAT_BC5_TYPELESS:
            result = ERenderFormat::RENDER_FORMAT_BC5_TYPELESS;

            break;
        case DXGI_FORMAT_BC5_UNORM:
            result = ERenderFormat::RENDER_FORMAT_BC5_UNORM;

            break;
        case DXGI_FORMAT_BC5_SNORM:
            result = ERenderFormat::RENDER_FORMAT_BC5_SNORM;

            break;
    }

    return result;
}

DirectX::TEX_DIMENSION Texture::GetTexDimension(const ZTextureMap::EDimensions dimension)
{
    DirectX::TEX_DIMENSION result;

    switch (dimension)
    {
        case ZTextureMap::EDimensions::DIMENSIONS_2D:
        case ZTextureMap::EDimensions::DIMENSIONS_CUBE:
            result = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;

            break;
        case ZTextureMap::EDimensions::DIMENSIONS_VOLUME:
            result = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE3D;

            break;
    }

    return result;
}

void Texture::GetRenderFormatDescription(SRenderFormatDesc* pDesc, const ERenderFormat& eFormat)
{
    switch (eFormat)
    {
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_UINT:
        case ERenderFormat::RENDER_FORMAT_R32G32B32A32_SINT:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 16;
            break;
        case ERenderFormat::RENDER_FORMAT_R32G32B32_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R32G32B32_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R32G32B32_UINT:
        case ERenderFormat::RENDER_FORMAT_R32G32B32_SINT:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 12;
            break;
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_UNORM:
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_UINT:
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_SNORM:
        case ERenderFormat::RENDER_FORMAT_R16G16B16A16_SINT:
        case ERenderFormat::RENDER_FORMAT_R32G32_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R32G32_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R32G32_UINT:
        case ERenderFormat::RENDER_FORMAT_R32G32_SINT:
        case ERenderFormat::RENDER_FORMAT_R32G8X24_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_D32_FLOAT_S8X24_UINT:
        case ERenderFormat::RENDER_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_X32_TYPELESS_G8X24_UINT:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 8;
            break;
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_UNORM:
        case ERenderFormat::RENDER_FORMAT_R10G10B10A2_UINT:
        case ERenderFormat::RENDER_FORMAT_R11G11B10_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UNORM_SRGB:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_UINT:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_SNORM:
        case ERenderFormat::RENDER_FORMAT_R8G8B8A8_SINT:
        case ERenderFormat::RENDER_FORMAT_R16G16_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R16G16_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R16G16_UNORM:
        case ERenderFormat::RENDER_FORMAT_R16G16_UINT:
        case ERenderFormat::RENDER_FORMAT_R16G16_SNORM:
        case ERenderFormat::RENDER_FORMAT_R16G16_SINT:
        case ERenderFormat::RENDER_FORMAT_R32_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_D32_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R32_FLOAT:
        case ERenderFormat::RENDER_FORMAT_R32_UINT:
        case ERenderFormat::RENDER_FORMAT_R32_SINT:
        case ERenderFormat::RENDER_FORMAT_R24G8_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_D24_UNORM_S8_UINT:
        case ERenderFormat::RENDER_FORMAT_R24_UNORM_X8_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_X24_TYPELESS_G8_UINT:
        case ERenderFormat::RENDER_FORMAT_R9G9B9E5_SHAREDEXP:
        case ERenderFormat::RENDER_FORMAT_R8G8_B8G8_UNORM:
        case ERenderFormat::RENDER_FORMAT_G8R8_G8B8_UNORM:
        case ERenderFormat::RENDER_FORMAT_INDEX_32:
        case ERenderFormat::RENDER_FORMAT_LE_X8R8G8B8_UNORM:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 4;
            break;
        case ERenderFormat::RENDER_FORMAT_R8G8_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R8G8_UNORM:
        case ERenderFormat::RENDER_FORMAT_R8G8_UINT:
        case ERenderFormat::RENDER_FORMAT_R8G8_SNORM:
        case ERenderFormat::RENDER_FORMAT_R8G8_SINT:
        case ERenderFormat::RENDER_FORMAT_R16_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R16_FLOAT:
        case ERenderFormat::RENDER_FORMAT_D16_UNORM:
        case ERenderFormat::RENDER_FORMAT_R16_UNORM:
        case ERenderFormat::RENDER_FORMAT_R16_UINT:
        case ERenderFormat::RENDER_FORMAT_R16_SNORM:
        case ERenderFormat::RENDER_FORMAT_R16_SINT:
        case ERenderFormat::RENDER_FORMAT_B5G6R5_UNORM:
        case ERenderFormat::RENDER_FORMAT_B5G5R5A1_UNORM:
        case ERenderFormat::RENDER_FORMAT_INDEX_16:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 2;
            break;
        case ERenderFormat::RENDER_FORMAT_R8_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_R8_UNORM:
        case ERenderFormat::RENDER_FORMAT_R8_UINT:
        case ERenderFormat::RENDER_FORMAT_R8_SNORM:
        case ERenderFormat::RENDER_FORMAT_R8_SINT:
        case ERenderFormat::RENDER_FORMAT_A8_UNORM:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 1;
            break;
        case ERenderFormat::RENDER_FORMAT_R1_UNORM:
            pDesc->nBlockWidth = 1;
            pDesc->nBlockHeight = 1;
            pDesc->nBytesPerBlock = 0;
            break;
        case ERenderFormat::RENDER_FORMAT_BC1_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_BC1_UNORM:
        case ERenderFormat::RENDER_FORMAT_BC1_UNORM_SRGB:
        case ERenderFormat::RENDER_FORMAT_BC4_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_BC4_UNORM:
        case ERenderFormat::RENDER_FORMAT_BC4_SNORM:
            pDesc->nBlockWidth = 4;
            pDesc->nBlockHeight = 4;
            pDesc->nBytesPerBlock = 8;
            break;
        case ERenderFormat::RENDER_FORMAT_BC2_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_BC2_UNORM:
        case ERenderFormat::RENDER_FORMAT_BC2_UNORM_SRGB:
        case ERenderFormat::RENDER_FORMAT_BC3_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_BC3_UNORM:
        case ERenderFormat::RENDER_FORMAT_BC3_UNORM_SRGB:
        case ERenderFormat::RENDER_FORMAT_BC5_TYPELESS:
        case ERenderFormat::RENDER_FORMAT_BC5_UNORM:
        case ERenderFormat::RENDER_FORMAT_BC5_SNORM:
            pDesc->nBlockWidth = 4;
            pDesc->nBlockHeight = 4;
            pDesc->nBytesPerBlock = 16;
            break;
    }
}

DirectX::TexMetadata Texture::GenerateTexMetadata()
{
    unsigned int width = textureMapHeader.nWidth;
    unsigned int height = textureMapHeader.nHeight;
    bool isCubeMap = static_cast<ZTextureMap::EDimensions>(textureMapHeader.nDimensions) == ZTextureMap::EDimensions::DIMENSIONS_CUBE;
    unsigned int arraySize = isCubeMap ? 6 : 1;
    int mipMapLevels = textureMapHeader.nNumMipLevels;
    unsigned int miscFlags = isCubeMap ? DirectX::TEX_MISC_FLAG::TEX_MISC_TEXTURECUBE : 0;
    const DXGI_FORMAT dxgiFormat = GetDXGIFormat(static_cast<ERenderFormat>(textureMapHeader.nFormat));
    DirectX::TEX_DIMENSION dimension = GetTexDimension(static_cast<ZTextureMap::EDimensions>(textureMapHeader.nDimensions));

    return DirectX::TexMetadata(width, height, 1, arraySize, mipMapLevels, miscFlags, 0, dxgiFormat, dimension);
}

void Texture::ConvertTextureToTGA(const std::string& outputFilePath)
{
    std::wstring outputFilePath2 = std::wstring(outputFilePath.begin(), outputFilePath.end());

    ConvertTextureToTGA(outputFilePath2);
}

void Texture::ConvertTextureToTGA(const std::wstring& outputFilePath)
{
    const DirectX::Image* image = nullptr;

    if (convertedScratchImage.GetPixelsSize() > 0)
    {
        image = convertedScratchImage.GetImage(0, 0, 0);
    }
    else
    {
        image = scratchImage.GetImage(0, 0, 0);
    }

    DirectX::ScratchImage scratchImage2;
    HRESULT result;

    if (DirectX::IsCompressed(texMetadata.format))
    {
        result = DirectX::Decompress(*image, DXGI_FORMAT_UNKNOWN, scratchImage2);

        if (FAILED(result))
        {
            std::cout << "Failed to decompress texture." << std::endl;

            return;
        }

        image = scratchImage2.GetImage(0, 0, 0);
    }

    result = DirectX::SaveToTGAFile(*image, outputFilePath.c_str(), &texMetadata);

    if (!SUCCEEDED(result))
    {
        std::cout << "Failed to convert texture to TGA. " << GetErrorMessage(result) << std::endl;
    }
}

std::string Texture::GetErrorMessage(HRESULT result)
{
	std::string message;

	switch (result)
	{
	case HRESULT_E_ARITHMETIC_OVERFLOW:
		message = "Arithmetic result exceeded 32 bits.";

		break;
	case HRESULT_E_NOT_SUPPORTED:
		message = "The request is not supported.";

		break;
	case HRESULT_E_HANDLE_EOF:
		message = "Reached the end of the file.";

		break;
	case HRESULT_E_INVALID_DATA:
		message = "The data is invalid.";

		break;
	case HRESULT_E_FILE_TOO_LARGE:
		message = "The file size exceeds the limit allowed and cannot be saved.";

		break;
	case HRESULT_E_CANNOT_MAKE:
		message = "The directory or file cannot be created.";

		break;
	case E_NOT_SUFFICIENT_BUFFER:
		message = "The data area passed to a system call is too small.";

		break;
	default:
		break;
	}

	return message;
}
