#include "system/windows_viewport_manager.hpp"
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

	WWindow::WWindow(ViewportInfo &info, HINSTANCE instance): name(info.name), hwnd(), running(false) {

		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = onCallback;
		wc.hInstance = instance;
		wc.hIcon = (HICON) LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 32, 32, 0);
		wc.hIconSm = (HICON) LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 16, 16, 0);
		wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
		wc.hbrBackground = ( HBRUSH) GetStockObject(BLACK_BRUSH);
		wc.lpszClassName = info.name.c_str();
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.cbWndExtra = sizeof(void*);

		if (!RegisterClassExA(&wc)) {
			String err = getLastError();
			System::log()->fatal(String("Couldn't init Windows class: ") + err);
			return;
		}

		DWORD style = WS_VISIBLE;

		if (info.hasHint(ViewportInfo::FULL_SCREEN))
			style |= WS_POPUP;

		else if(!info.hasHint(ViewportInfo::NO_MENU)) {

			style |= WS_SYSMENU | WS_CAPTION;

			if (!info.hasHint(ViewportInfo::NOT_RESIZABLE))
				 style |= WS_SIZEBOX | WS_MAXIMIZEBOX;

			if (!info.hasHint(ViewportInfo::NOT_MINIMIZABLE))
				style |= WS_MINIMIZEBOX;
		}

		Vec2u maxSize = { u32(GetSystemMetrics(SM_CXSCREEN)), u32(GetSystemMetrics(SM_CYSCREEN)) };

		for (usz i = 0; i < 2; ++i)
			if (info.size[i] == 0 || info.size[i] >= maxSize[i])
				info.size[i] = maxSize[i];

		hwnd = CreateWindowExA(
			WS_EX_APPWINDOW, wc.lpszClassName, wc.lpszClassName, style,
			info.offset[0], info.offset[1], info.size[0], info.size[1],
			NULL, NULL, instance, NULL
		);

		if(hwnd == NULL) {
			String err = getLastError();
			System::log()->fatal(String("Couldn't create window: ") + err);
			return;
		}

		SetWindowLongPtrA(hwnd, 0, (LONG_PTR)this);
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	struct WViewportThreadData {
		ViewportInfo &info;
		std::mutex m;
		std::condition_variable cv;
		bool ready = false;
	};

	void WViewportManager::pull(WViewportThreadData *data) {

		//TODO: In the future, Modify it so that only 1 thread is used max
		//		Right now there's one per window

		WWindow *w = new WWindow(data->info, instance);
		hwnd.push_back(w);

		w->running = data->ready = true;
		data->cv.notify_one();

		MSG msg, *msgp = &msg;

		while (w->running) {

			while (PeekMessageA(msgp, w->hwnd, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(msgp);
				DispatchMessageA(msgp);
			}

			if (msg.message == WM_QUIT)
				break;
		}

		w->hwnd = NULL;
		destroy(*find(w->name));
	}

	void WViewportManager::add(ViewportInfo &info) {

		WViewportThreadData data { info };
		std::future<void> thr = std::async(&WViewportManager::pull, this, &data);

		std::unique_lock<std::mutex> lk(data.m);
		data.cv.wait(lk, [&data] { return data.ready; });

		WWindow *wnd = hwnd[hwnd.size() - 1];
		wnd->thr = std::move(thr);
	}
	
	void WViewportManager::del(const ViewportInfo &info) {
		delete hwnd[info.id];
		hwnd.erase(hwnd.begin() + info.id);
	}

	WWindow::~WWindow() {
		if (hwnd) {
			DestroyWindow(hwnd);
			running = false;
			thr.wait();
		}
	}

	LRESULT CALLBACK onCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch (message) {

			case WM_CREATE:
				srand(u32(time(0)));
				break;

			case WM_PAINT:

				{
					RECT r;
					GetClientRect(hwnd, &r);

					for (usz i = 0; i < rand() % 256 && r.right && r.bottom; ++i)
						SetPixel(GetDC(hwnd), 
								 rand() % r.right, rand() % r.bottom, 
								 RGB(rand() % 200 + 56, rand() % 200 + 56, rand() % 200 + 56)
						);
				}
				return NULL;

			case WM_CLOSE:
				{
					auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);
					ptr->running = false;
				}
				break;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

}