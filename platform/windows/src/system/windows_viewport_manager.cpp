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

		WWindow *w = new WWindow(data->info, instance);
		hwnd.push_back(w);

		w->running = data->ready = true;
		data->cv.notify_one();

		MSG msg, *msgp = &msg;

		while (w->running) {

			while (GetMessageA(msgp, w->hwnd, 0U, 0U)) {

				w->info->fence.lock();

				TranslateMessage(msgp);
				DispatchMessageA(msgp);

				w->info->fence.unlock();

				if (msg.hwnd == NULL)
					break;
			}
		}

		w->hwnd = NULL;
		destroy(w->info);
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

					//TODO: This should be done by a callback!

					u16 *pixelBuffer = (u16*)0x20000;
					constexpr usz w = 160, h = 144;

					RECT r;
					GetClientRect(hwnd, &r);

					PAINTSTRUCT paintStruct;
					HDC dc = BeginPaint(hwnd, &paintStruct);

					List<u32> pixels(r.bottom * r.right);

					for(usz j = 0; j < r.bottom; ++j)
						for (usz i = 0; i < r.right; ++i) {

							usz x = usz(f64(i) / (r.right - 1) * (w - 1));
							usz y = usz(f64(j) / (r.bottom - 1) * (h - 1));

							u16 &pix = pixelBuffer[x + y * w];

							pixels[j * r.right + i] = RGB((pix << 3) & 0xFF, (pix >> 2) & ~7, (pix >> 7) & ~7);
						}

					BITMAPINFOHEADER bi;
					ZeroMemory(&bi, sizeof(bi));
					bi.biSize = sizeof(bi);
					bi.biWidth = r.right;
					bi.biHeight = r.bottom;
					bi.biPlanes = 1;
					bi.biBitCount = sizeof(pixels[0]) * 8;
					bi.biCompression = 0;  //RGB
					bi.biSizeImage = DWORD(pixels.size() * sizeof(pixels[0]));

					SetDIBitsToDevice(
						dc, 0, 0, r.right, r.bottom, 0, 0, 0, r.bottom, 
						pixels.data(), (LPBITMAPINFO)&bi, DIB_RGB_COLORS
					);

					EndPaint(hwnd, &paintStruct);
				}
				return NULL;

			case WM_CLOSE:
				{
					auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);
					ptr->running = false;
				}
				break;

			case WM_SIZE:
				{
					auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);

					if (!ptr)
						break;

					RECT r;
					GetClientRect(hwnd, &r);

					ptr->info->size = { u32(r.right - r.left), u32(r.bottom - r.top) };
				}
				break;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void WViewportManager::redraw(const ViewportInfo *info) {
		RedrawWindow(hwnd[info->id]->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}

}