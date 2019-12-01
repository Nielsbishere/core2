#include "system/windows_viewport_manager.hpp"
#include "system/viewport_interface.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "input/windows_input.hpp"
#include "utils/timer.hpp"
#include <comdef.h>
#include <Windows.h>
#include <stdio.h>

namespace oic {
	bool Mouse::isSupported(Handle) const { return true; }
	bool Keyboard::isSupported(Handle) const { return true; }
}

namespace oic::windows {

	String getLastError() {
		HRESULT hr = GetLastError();
		_com_error err(hr);
		return err.ErrorMessage();
	}

	LRESULT CALLBACK onCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	WViewportManager::WViewportManager(): instance(GetModuleHandleA(NULL)) { }
	
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
			style |= WS_POPUPWINDOW;

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

		RECT r{};
		GetClientRect(hwnd, &r);
		info->size = { u32(r.right - r.left), u32(r.bottom - r.top) };

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
			w->info->vinterface->resize(w->info, w->info->size);
		}

		ShowWindow(w->hwnd, SW_SHOW);

		if (data->info->hasHint(ViewportInfo::HANDLE_INPUT)) {

			//Register for both mice and keyboard
			RAWINPUTDEVICE test[] = {
				{
					0x01,
					0x06,
					RIDEV_DEVNOTIFY,
					w->hwnd
				},
				{
					0x01,
					0x02,
					RIDEV_DEVNOTIFY,
					w->hwnd
				}
			};

			oicAssert("Couldn't register input devices", RegisterRawInputDevices(test, 2, sizeof(RAWINPUTDEVICE)));

		}

		w->running = data->ready = true;
		data->cv.notify_one();

		MSG msg, *msgp = &msg;
		bool quit = false;

		ns last{};

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

				//Update interface

				ns now = oic::Timer::now();

				if (w->info->vinterface)
					w->info->vinterface->update(w->info, last ? (now - last) / 1'000'000'000.0 : 0);

				last = oic::Timer::now();

				//Update input

				for (auto dvc : w->info->devices) {

					for (ButtonHandle i = 0, j = ButtonHandle(dvc->getButtonCount()); i < j; ++i)
						if (dvc->getState(i) == 0x2 /* released */)
							dvc->setPreviousState(i, false);
						else if (dvc->getState(i) == 0x1 /* pressed */)
							dvc->setPreviousState(i, true);

					for (AxisHandle i = 0, j = dvc->getAxisCount(); i < j; ++i)
						dvc->setPreviousAxis(i, dvc->getCurrentAxis(i));
				}

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

			case WM_INPUT: {

				u32 size{};

				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

				Buffer buf(size);

				oicAssert("Couldn't read input", GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf.data(), &size, sizeof(RAWINPUTHEADER)) == size);

				RAWINPUT *data = (RAWINPUT*)buf.data();

				RECT rect;
				GetWindowRect(hwnd, &rect);

				//For some reason it's 8 pixels off?

				int menuSize = GetSystemMetrics(SM_CYMENU);

				rect.left += 8;
				rect.top += 8 + menuSize;

				if(data->header.hDevice)
					if (auto *ptr = (WWindow *)GetWindowLongPtrA(hwnd, 0)) {

						InputDevice *&dvc = ptr->devices[data->header.hDevice];

						if (dvc->getType() == InputDevice::Type::KEYBOARD) {

							auto &keyboardDat = data->data.keyboard;

							usz id = WKey::idByValue(WKey::_E(keyboardDat.VKey));

							//TODO: Keyboard should initialize CAPS, SHIFT, ALT if they get changed or on start/switch

							//Only send recognized keys

							if (id != WKey::count) {

								usz keyCode = Key::idByName(WKey::nameById(id));
								bool isKeyDown = !(keyboardDat.Flags & 1);

								bool pressed = dvc->getCurrentState(ButtonHandle(keyCode));
								dvc->setState(ButtonHandle(keyCode), isKeyDown);

								if (ptr->info->vinterface && pressed != isKeyDown) {

									if (isKeyDown)
										ptr->info->vinterface->onInputActivate(ptr->info, dvc, InputDevice::Handle(keyCode));
									else 
										ptr->info->vinterface->onInputDeactivate(ptr->info, dvc, InputDevice::Handle(keyCode));
								};
							}

							return 0;

						} else {

							auto &mouseDat = data->data.mouse;

							for (usz i = 0; i < 5; ++i) {

								if (mouseDat.usButtonFlags & (1 << (i << 1))) {

									dvc->setPreviousState(ButtonHandle(i), dvc->getPreviousState(ButtonHandle(i)));
									dvc->setState(ButtonHandle(i), true);

									if(ptr->info->vinterface)
										ptr->info->vinterface->onInputActivate(ptr->info, dvc, InputDevice::Handle(i));
								}

								else if (mouseDat.usButtonFlags & (2 << (i << 1))) {

									dvc->setPreviousState(ButtonHandle(i), true);
									dvc->setState(ButtonHandle(i), false);

									if(ptr->info->vinterface)
										ptr->info->vinterface->onInputDeactivate(ptr->info, dvc, InputDevice::Handle(i));
								}

							}

							if (mouseDat.usButtonFlags & RI_MOUSE_WHEEL)
								dvc->setAxis(MouseAxis::AXIS_WHEEL, i16(mouseDat.usButtonData) / f64(i16_MAX));

							if (mouseDat.usFlags & MOUSE_MOVE_ABSOLUTE) {

								f64 x = f64(mouseDat.lLastX) - rect.left, y = f64(mouseDat.lLastY) - rect.top;

								dvc->setAxis(MouseAxis::AXIS_DELTA_X, dvc->getCurrentAxis(MouseAxis::AXIS_X) - x);
								dvc->setAxis(MouseAxis::AXIS_DELTA_Y, dvc->getCurrentAxis(MouseAxis::AXIS_Y) - y);
								dvc->setAxis(MouseAxis::AXIS_X, x);
								dvc->setAxis(MouseAxis::AXIS_Y, y);

							} else {

								dvc->setAxis(MouseAxis::AXIS_DELTA_X, mouseDat.lLastX);
								dvc->setAxis(MouseAxis::AXIS_DELTA_Y, mouseDat.lLastY);

								POINT point;
								GetCursorPos(&point);

								dvc->setAxis(MouseAxis::AXIS_X, f64(point.x) - rect.left);
								dvc->setAxis(MouseAxis::AXIS_Y, f64(point.y) - rect.top);
							}
						}

					}

				return DefRawInputProc(&data, 1, sizeof(*data));
			}

			case WM_INPUT_DEVICE_CHANGE: {

				if (auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0)) {

					RID_DEVICE_INFO deviceInfo{};
					u32 size = sizeof(deviceInfo);
					oicAssert("Couldn't get raw input device", GetRawInputDeviceInfoA((HANDLE)lParam, RIDI_DEVICEINFO, &deviceInfo, &size));

					auto *info = ptr->info;
					bool isAdded = wParam == GIDC_ARRIVAL;
					InputDevice *&dvc = ptr->devices[(HANDLE)lParam];

					oicAssert("Device change was already notified", bool(dvc) != isAdded);

					if (isAdded) {

						bool isKeyboard = deviceInfo.dwType == RIM_TYPEKEYBOARD;

						if (isKeyboard)
							info->devices.push_back(dvc = new Keyboard());
						else
							info->devices.push_back(dvc = new Mouse());

						if (info->vinterface)
							info->vinterface->onDeviceConnect(info, dvc);

						RAWINPUTDEVICE device {
							0x01,
							u16(isKeyboard ? 0x06 : 0x02),
							0x0,
							hwnd
						};

						if (!isKeyboard) {
							POINT point;
							GetCursorPos(&point);
							dvc->setAxis(MouseAxis::AXIS_X, point.x);
							dvc->setAxis(MouseAxis::AXIS_Y, point.y);
						}

						oicAssert("Couldn't create raw input device", RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE)));

					} else {

						if (info->vinterface)
							info->vinterface->onDeviceRemoval(info, dvc);

						info->devices.erase(std::find(ptr->info->devices.begin(), ptr->info->devices.end(), dvc));
						ptr->devices.erase((HANDLE)lParam);
						delete dvc;
					}

				}

				return 0;
			}

			case WM_PAINT:

				if (auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0)) {

					if (ptr->running && ptr->info->vinterface)
						ptr->info->vinterface->render(ptr->info);
				}

				return NULL;

			case WM_GETMINMAXINFO: {
				LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
				lpMMI->ptMinTrackSize.x = 256;
				lpMMI->ptMinTrackSize.y = 256;
				break;
			}

			case WM_CLOSE: {
				auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);
				ptr->running = false;
				break;
			}

			case WM_DESTROY:
				break;

			case WM_SIZE: {

				auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);

				if (!ptr)
					break;

				RECT r;
				GetClientRect(hwnd, &r);

				ptr->info->size = { u32(r.right - r.left), u32(r.bottom - r.top) };

				if(ptr->info->vinterface)
					ptr->info->vinterface->resize(ptr->info, ptr->info->size);

				break;
			}

			case WM_MOVE: {
					
				auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);

				if (!ptr)
					break;

				RECT r;
				GetClientRect(hwnd, &r);

				ptr->info->offset = { r.left, r.top };
				break;
			}
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void WViewportManager::redraw(const ViewportInfo *info) {
		RedrawWindow(hwnd[info->id]->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}

}