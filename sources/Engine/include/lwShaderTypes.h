//
#pragma once

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwStdInc.h"

LW_BEGIN
	enum lwVertexDeclarationTypesEnum {
		// vertex declaration
		__begin_decl_type = 0,
		VDT_PD = 0,
		VDT_PDT0,
		VDT_PDT1,
		VDT_PDT2,
		VDT_PDT3,
		VDT_PNT0,
		VDT_PND,
		VDT_PNDT0,
		VDT_PNDT1,
		VDT_PNDT2,
		VDT_PNDT3,

		VDT_PU4NT0,
		VDT_PB1U4NT0,
		VDT_PB2U4NT0,
		VDT_PB3U4NT0,

		VDT_EFF_134,
		VDT_EFF_2,
		VDT_EFF_SHADE,
		VDT_EFF_MINIMAP,

		VDT_USER_DEFINE = 0xff,

		VDT_INVALID = LW_INVALID_INDEX
	};

	enum lwVertexShaderTypesEnum {
		// vertex shader 
		__begin_vs_type = 0,

		VST_PU4NT0_LD,
		VST_PB1U4NT0_LD,
		VST_PB2U4NT0_LD,
		VST_PB3U4NT0_LD,

		VST_PNT0_LD,
		VST_PNT0_LD_TT0,
		VST_PNT0_TT0,

		VST_PNDT0,
		VST_PNDT0_TT0,
		VST_PNDT0_LD,
		VST_PNDT0_LD_TT0,


		VST_USER_DEFINE = 0xff,

		VST_INVALID = LW_INVALID_INDEX,
	};

	enum lwVertexShaderLightTypeEnum {
		VSLT_DIRECTIONAL = 3,
		VSLT_INVALID = LW_INVALID_INDEX,
	};

	enum lwVertexShaderAnimTypeEnum {
		VSAT_NULL = 0x0000,

		VSAT_VERTEXMATRIX = VSAT_NULL, // donot need vs support

		VSAT_TEXTURETRANSFORM0 = 0x0010,
		VSAT_TEXTURETRANSFORM1 = 0x0020,
		VSAT_TEXTURETRANSFORM2 = 0x0030,
		VSAT_TEXTURETRANSFORM3 = 0x0040,

		VSAT_INVALID = LW_INVALID_INDEX
	};

	struct lwShaderDeclQueryInfo {
		DWORD decl_type;
		DWORD light_type;
		DWORD anim_type;
	};

	struct lwShaderDeclCreateInfo {
		DWORD shader_id;
		DWORD decl_type;
		DWORD light_type;
		DWORD anim_type;
	};

	inline lwShaderDeclCreateInfo SDCI_VALUE(DWORD shader, DWORD decl, DWORD light, DWORD anim) {
		lwShaderDeclCreateInfo i;
		i.shader_id = shader;
		i.decl_type = decl;
		i.light_type = light;
		i.anim_type = anim;
		return i;
	}

LW_END
