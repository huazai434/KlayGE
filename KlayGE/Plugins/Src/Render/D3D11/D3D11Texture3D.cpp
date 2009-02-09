// D3D11Texture3D.cpp
// KlayGE D3D11 3D������ ʵ���ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <d3dx11.h>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx11.lib")
#else
	#pragma comment(lib, "d3dx11.lib")
#endif
#endif

namespace KlayGE
{
	D3D11Texture3D::D3D11Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D11Texture(TT_3D, sample_count, sample_quality, access_hint)
	{
		numMipMaps_ = numMipMaps;
		format_		= format;
		widthes_.assign(1, width);
		heights_.assign(1, height);
		depthes_.assign(1, depth);

		bpp_ = NumFormatBits(format);

		desc_.Width = width;
		desc_.Height = height;
		desc_.Depth = depth;
		desc_.MipLevels = numMipMaps_;
		desc_.Format = D3D11Mapping::MappingFormat(format_);

		this->GetD3DFlags(desc_.Usage, desc_.BindFlags, desc_.CPUAccessFlags, desc_.MiscFlags);

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data(numMipMaps_);
		if (init_data != NULL)
		{
			for (int i = 0; i < numMipMaps_; ++ i)
			{
				subres_data[i].pSysMem = init_data[i].data;
				subres_data[i].SysMemPitch = init_data[i].row_pitch;
				subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
			}
		}

		ID3D11Texture3D* d3d_tex;
		TIF(d3d_device_->CreateTexture3D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture3D_ = MakeCOMPtr(d3d_tex);

		if (access_hint_ & EAH_GPU_Read)
		{
			ID3D11ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(d3dTexture3D_.get(), NULL, &d3d_sr_view);
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		this->UpdateParams();
	}

	uint32_t D3D11Texture3D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widthes_[level];
	}

	uint32_t D3D11Texture3D::Height(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return heights_[level];
	}

	uint32_t D3D11Texture3D::Depth(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return depthes_[level];
	}

	void D3D11Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture3D& other(*checked_cast<D3D11Texture3D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0))
			&& (this->Depth(0) == target.Depth(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			d3d_imm_ctx_->CopyResource(other.D3DTexture().get(), d3dTexture3D_.get());
		}
		else
		{
			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = NULL;
			info.pDstBox = NULL;
			info.SrcFirstMip = D3D11CalcSubresource(0, 0, 1);
			info.DstFirstMip = D3D11CalcSubresource(0, 0, 1);
			info.NumMips = std::min(this->NumMipMaps(), target.NumMipMaps());
			info.SrcFirstElement = 0;
			info.DstFirstElement = 0;
			info.NumElements = 0;
			info.Filter = D3DX11_FILTER_LINEAR;
			info.MipFilter = D3DX11_FILTER_LINEAR;
			if (IsSRGB(format_))
			{
				info.Filter |= D3DX11_FILTER_SRGB_IN;
				info.MipFilter |= D3DX11_FILTER_SRGB_IN;
			}
			if (IsSRGB(target.Format()))
			{
				info.Filter |= D3DX11_FILTER_SRGB_OUT;
				info.MipFilter |= D3DX11_FILTER_SRGB_OUT;
			}

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture3D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture3D::CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture3D& other(*checked_cast<D3D11Texture3D*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = src_zOffset;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = src_zOffset = src_depth;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DTexture().get(), D3D11CalcSubresource(level, 0, 1),
				dst_xOffset, dst_yOffset, 0, d3dTexture3D_.get(), D3D11CalcSubresource(level, 0, 1), &src_box);
		}
		else
		{
			D3D11_BOX src_box, dst_box;

			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = src_zOffset;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = src_zOffset + src_depth;

			dst_box.left = dst_xOffset;
			dst_box.top = dst_yOffset;
			dst_box.front = dst_zOffset;
			dst_box.right = dst_xOffset + dst_width;
			dst_box.bottom = dst_yOffset + dst_height;
			dst_box.back = dst_zOffset + dst_depth;

			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = &src_box;
			info.pDstBox = &dst_box;
			info.SrcFirstMip = D3D11CalcSubresource(level, 0, 1);
			info.DstFirstMip = D3D11CalcSubresource(level, 0, 1);
			info.NumMips = 1;
			info.SrcFirstElement = 0;
			info.DstFirstElement = 0;
			info.NumElements = 0;
			info.Filter = D3DX11_FILTER_LINEAR;
			info.MipFilter = D3DX11_FILTER_LINEAR;
			if (IsSRGB(format_))
			{
				info.Filter |= D3DX11_FILTER_SRGB_IN;
				info.MipFilter |= D3DX11_FILTER_SRGB_IN;
			}
			if (IsSRGB(target.Format()))
			{
				info.Filter |= D3DX11_FILTER_SRGB_OUT;
				info.MipFilter |= D3DX11_FILTER_SRGB_OUT;
			}

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture3D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture3D::Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3dTexture3D_.get(), D3D11CalcSubresource(level, 0, 1), D3D11Mapping::Mapping(tma, type_, access_hint_, numMipMaps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + (z_offset * mapped.DepthPitch + y_offset * mapped.RowPitch + x_offset) * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
		slice_pitch = mapped.DepthPitch;
	}

	void D3D11Texture3D::Unmap3D(int level)
	{
		d3d_imm_ctx_->Unmap(d3dTexture3D_.get(), D3D11CalcSubresource(level, 0, 1));
	}

	void D3D11Texture3D::BuildMipSubLevels()
	{
		if (d3d_sr_view_)
		{
			if (!(desc_.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS))
			{
				desc_.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

				ID3D11Texture3D* d3d_tex;
				TIF(d3d_device_->CreateTexture3D(&desc_, NULL, &d3d_tex));

				d3d_imm_ctx_->CopyResource(d3d_tex, d3dTexture3D_.get());

				d3dTexture3D_ = MakeCOMPtr(d3d_tex);

				ID3D11ShaderResourceView* d3d_sr_view;
				d3d_device_->CreateShaderResourceView(d3dTexture3D_.get(), NULL, &d3d_sr_view);
				d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
			}

			d3d_imm_ctx_->GenerateMips(d3d_sr_view_.get());
		}
		else
		{
			D3DX11FilterTexture(d3d_imm_ctx_.get(), d3dTexture3D_.get(), 0, D3DX11_FILTER_LINEAR);
		}
	}

	void D3D11Texture3D::UpdateParams()
	{
		d3dTexture3D_->GetDesc(&desc_);

		numMipMaps_ = static_cast<uint16_t>(desc_.MipLevels);
		BOOST_ASSERT(numMipMaps_ != 0);

		widthes_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depthes_.resize(numMipMaps_);
		widthes_[0] = desc_.Width;
		heights_[0] = desc_.Height;
		depthes_[0] = desc_.Depth;
		for (uint16_t level = 1; level < numMipMaps_; ++ level)
		{
			widthes_[level] = widthes_[level - 1] / 2;
			heights_[level] = heights_[level - 1] / 2;
			depthes_[level] = depthes_[level - 1] / 2;
		}

		format_ = D3D11Mapping::MappingFormat(desc_.Format);
		bpp_	= NumFormatBits(format_);
	}
}