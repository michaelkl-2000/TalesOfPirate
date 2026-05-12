#pragma once

#include <algorithm>
#include <cstdint>
#define NOMINMAX
#include <windows.h>
#include <d3d9.h>

#include "FontRender.h"

bool GetIntersectRect(RECT* pdest, RECT* psrc, RECT* pclip);

namespace ui {
#define UI_STATE_NOCLIP		0
#define UI_STATE_CLIP		1

	class UIClip {
	public:
		static UIClip* Instance() {
			if (_pClip) {
				return _pClip;
			}
			_pClip = new UIClip;
			return _pClip;
		}

		void SetClipRect(int x, int y, int w, int h) {
			_rectClip.left = x;
			_rectClip.top = y;
			_rectClip.right = x + w;
			_rectClip.bottom = y + h;
			_state = UI_STATE_CLIP;
		}

		void Reset() {
			_state = UI_STATE_NOCLIP;
		}

		std::uint8_t GetClipState() {
			return _state;
		}

		RECT& GetClipRect() {
			return _rectClip;
		}

		static UIClip* GetCliper() {
			return _pClip;
		}

		bool GetIntersectRect(RECT* pdest, RECT* psrc, std::uint8_t* byFill = nullptr);

	protected:
		UIClip();
		~UIClip();

	private:
		std::uint8_t _state;
		RECT _rectClip;
		static UIClip* _pClip;
	};
}

//===================================================================
// FVF и флаги — сохранены для совместимости со старыми call-sites.
#define D3DFVF_FONT   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define D3DFVF_3DFONT (D3DFVF_XYZB1  | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#ifndef MPFONT_BOLD
#define MPFONT_BOLD   0x0001
#define MPFONT_ITALIC 0x0002
#define MPFONT_UNLINE 0x0004
#endif

// CMPFont — alias-через-наследование к FontRender. Старые call-sites
// продолжают работать без изменений: pointer/container типа CMPFont*
// разрешаются как FontRender*, все методы приходят из базы.
class CMPFont : public FontRender {
};
