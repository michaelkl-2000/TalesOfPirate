//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"


namespace Corsairs::Engine::Render {
	class DirectoryBrowser : public IDirectoryBrowser {
		LW_STD_DECLARATION()

	private:
		DirectoryBrowserProc _proc;
		void* _param;

	private:
		LW_RESULT _Go(std::string_view file, DWORD flag);

	public:
		DirectoryBrowser();

		void SetBrowseProc(DirectoryBrowserProc proc, void* param) {
			_proc = proc;
			_param = param;
		}

		LW_RESULT Browse(std::string_view file, DWORD flag);
	};


} // namespace Corsairs::Engine::Render
