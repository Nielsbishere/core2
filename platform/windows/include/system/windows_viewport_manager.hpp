#pragma once
#include "system/viewport_manager.hpp"
#include <condition_variable>
#include <future>

struct HINSTANCE__;
using HINSTANCE = HINSTANCE__ *;

struct HWND__;
using HWND = HWND__ *;

namespace oic::windows {

	struct WWindow {

		ViewportInfo *info;
		HWND hwnd;
		bool running;
		std::future<void> thr;

		WWindow(ViewportInfo *info, HINSTANCE instance);
		~WWindow();

		WWindow(WWindow &&w) = delete;
		WWindow &operator=(WWindow &&w) = delete;
		WWindow(const WWindow&) = delete;
		WWindow &operator=(const WWindow&) = delete;
	};

	class WViewportManager : public ViewportManager {

	public:

		WViewportManager();
		~WViewportManager() = default;

		virtual void add(ViewportInfo *info) final override;
		virtual void del(const ViewportInfo *info) final override;

		struct WViewportThreadData {
			ViewportInfo *info;
			std::mutex m;
			std::condition_variable cv;
			bool ready = false;
		};

		void pull(WViewportThreadData *data);

		virtual void redraw(const ViewportInfo *info) final override;

	private:

		HINSTANCE instance;
		List<WWindow*> hwnd;

	};

}