#include "system/windows_viewport_manager.hpp"
#include "system/viewport_interface.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include <comdef.h>
#include <Windows.h>
#include <stdio.h>

namespace oic::windows {

	String getLastError() {
		HRESULT hr = GetLastError();
		_com_error err(hr);
		return err.ErrorMessage();
	}

	LRESULT CALLBACK onCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	WViewportManager::WViewportManager(): instance(GetModuleHandleA(NULL)) {}
	
	WViewportManager::~WViewportManager() {

		for (ViewportInfo *info : *this)
			del(info);
	}

	WWindow::WWindow(ViewportInfo *info, HINSTANCE instance): hwnd(), running(false), info(info) {

		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = onCallback;
		wc.hInstance = instance;
		wc.hIcon = (HICON) LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 32, 32, 0);
		wc.hIconSm = (HICON) LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 16, 16, 0);
		wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
		wc.hbrBackground = ( HBRUSH) GetStockObject(BLACK_BRUSH);
		wc.lpszClassName = info->name.c_str();
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.cbWndExtra = sizeof(void*);

		if (!RegisterClassExA(&wc)) {
			String err = getLastError();
			System::log()->fatal(String("Couldn't init Windows class: ") + err);
			return;
		}

		DWORD style = WS_VISIBLE;

		if (info->hasHint(ViewportInfo::FULL_SCREEN))
			style |= WS_POPUP;

		else if(!info->hasHint(ViewportInfo::NO_MENU)) {

			style |= WS_SYSMENU | WS_CAPTION;

			if (!info->hasHint(ViewportInfo::NOT_RESIZABLE))
				 style |= WS_SIZEBOX | WS_MAXIMIZEBOX;

			if (!info->hasHint(ViewportInfo::NOT_MINIMIZABLE))
				style |= WS_MINIMIZEBOX;
		}

		Vec2u maxSize = { u32(GetSystemMetrics(SM_CXSCREEN)), u32(GetSystemMetrics(SM_CYSCREEN)) };

		for (usz i = 0; i < 2; ++i)
			if (info->size[i] == 0 || info->size[i] >= maxSize[i])
				info->size[i] = maxSize[i];

		hwnd = CreateWindowExA(
			WS_EX_APPWINDOW, wc.lpszClassName, wc.lpszClassName, style,
			info->offset[0], info->offset[1], info->size[0], info->size[1],
			NULL, NULL, instance, NULL
		);

		if(hwnd == NULL) {
			String err = getLastError();
			System::log()->fatal(String("Couldn't create window: ") + err);
			return;
		}

		SetWindowLongPtrA(hwnd, 0, (LONG_PTR)this);
		UpdateWindow(hwnd);
	}

	struct WViewportThreadData {
		ViewportInfo &info;
		std::mutex m;
		std::condition_variable cv;
		bool ready = false;
	};

	void WViewportManager::pull(WViewportThreadData *data) {

		WWindow *w = new WWindow(data->info, instance);
		hwnd.push_back(w);

		if (w->info->vinterface) {
			w->info->vinterface->init(w->info);
			w->info->vinterface->resize(w->info->size);
		}

		ShowWindow(w->hwnd, SW_SHOW);

		w->running = data->ready = true;
		data->cv.notify_one();

		MSG msg, *msgp = &msg;
		bool quit = false;

		while (w->running) {

			while (GetMessageA(msgp, w->hwnd, 0U, 0U)) {

				w->info->fence.lock();

				TranslateMessage(msgp);
				DispatchMessageA(msgp);

				w->info->fence.unlock();

				//Exit called from main thread
				if (msg.message == WM_DESTROY) {
					quit = true;
					DestroyWindow(w->hwnd);
					continue;
				}

				//Exit called from current thread
				if (msg.hwnd == NULL)
					break;
			}
		}

		w->hwnd = NULL;

		if (!quit)
			destroy(w->info);
		else
			destroyTopLevel(w->info);
	}

	void WViewportManager::add(ViewportInfo *info) {

		WViewportThreadData data { info };
		std::future<void> thr = std::async(&WViewportManager::pull, this, &data);

		std::unique_lock<std::mutex> lk(data.m);
		data.cv.wait(lk, [&data] { return data.ready; });

		WWindow *wnd = hwnd[hwnd.size() - 1];
		wnd->thr = std::move(thr);
	}
	
	void WViewportManager::del(const ViewportInfo *info) {
		delete hwnd[info->id];
		hwnd.erase(hwnd.begin() + info->id);
	}

	WWindow *WViewportManager::get(ViewportInfo *info) {
		return hwnd[info->id];
	}

	WWindow::~WWindow() {
		if (hwnd) {
			running = false;
			PostMessageA(hwnd, WM_DESTROY, NULL, NULL);
			thr.wait();
		}
	}

	LRESULT CALLBACK onCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch (message) {

			case WM_CREATE:
				break;

			case WM_PAINT:

				if(auto *ptr = (WWindow*) GetWindowLongPtrA(hwnd, 0))
					if(ptr->running)
						ptr->info->vinterface->render();

				return NULL;

			case WM_CLOSE:
				{
					auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);
					ptr->running = false;
				}
				break;

			case WM_DESTROY:
				break;

			case WM_SIZE:
				{
					auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);

					if (!ptr)
						break;

					RECT r;
					GetClientRect(hwnd, &r);

					ptr->info->size = { u32(r.right - r.left), u32(r.bottom - r.top) };
					ptr->info->vinterface->resize(ptr->info->size);
				}
				break;

			//TODO: WM_MOVE
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void WViewportManager::redraw(const ViewportInfo *info) {
		RedrawWindow(hwnd[info->id]->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}

}