#include "system/windows_viewport_manager.hpp"
#include "system/viewport_interface.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "input/windows_input.hpp"
#include "types/mat.hpp"
#include "utils/timer.hpp"

#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <dwrite.h>
#include <ShellScalingApi.h>
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "Shcore.lib")

namespace oic {

	bool Mouse::isSupported(Handle) const { return true; }
	bool Keyboard::isSupported(Handle) const { return true; }

}

namespace oic::windows {

	String getLastError() {
		HRESULT hr = GetLastError();
		return std::system_error(hr, std::system_category()).what();
	}

	LRESULT CALLBACK onCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	struct ForeachMonitorData {
		IDWriteFactory *factory;
		List<Monitor> *monitors;
	};

	BOOL foreachMonitor(HMONITOR monitor, HDC, LPRECT rect, ForeachMonitorData *data) {

		//Get settings from clear type (Windows' built-in font renderer)

		IDWriteRenderingParams *renderParams{};
		HRESULT hr;

		if (FAILED(hr = data->factory->CreateMonitorRenderingParams(monitor, &renderParams)))
			oic::System::log()->fatal(std::system_error(hr, std::system_category()).what());

		DWRITE_PIXEL_GEOMETRY geometry = renderParams->GetPixelGeometry();
		f32 gamma = renderParams->GetGamma();
		f32 contrast = renderParams->GetEnhancedContrast();

		renderParams->Release();

		//Get sample positions from pixel order

		Array<Vec2f32, 3> samples{};

		if (geometry == DWRITE_PIXEL_GEOMETRY_BGR)
			samples = { Vec2f32(1, 0), Vec2f32(0, 0), Vec2f32(-1, 0) };

		else if (geometry == DWRITE_PIXEL_GEOMETRY_RGB)
			samples = { Vec2f32(-1, 0), Vec2f32(0, 0), Vec2f32(1, 0) };

		//Get display settings

		MONITORINFOEXA monInfo{};
		monInfo.cbSize = sizeof(monInfo);

		bool hasInfo = GetMonitorInfoA(monitor, &monInfo);

		oicAssert("Couldn't get monitor info " + getLastError(), hasInfo);

		DEVMODEA mode{};
		bool hasDeviceSettings = EnumDisplaySettingsA(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &mode);

		oicAssert("Couldn't get display settings " + GetLastError(), hasDeviceSettings);

		Orientation orientation{};	//Physical orientation

		if (mode.dmDisplayOrientation == DMDO_90)
			orientation = Orientation::PORTRAIT;

		else if (mode.dmDisplayOrientation == DMDO_180)
			orientation = Orientation::LANDSCAPE_FLIPPED;

		else if (mode.dmDisplayOrientation == DMDO_270)
			orientation = Orientation::PORTRAIT_FLIPPED;

		i32 refreshRate = mode.dmDisplayFrequency;

		//Get physical size

		u32 dpiX, dpiY;

		if(FAILED(hr = GetDpiForMonitor(monitor, MDT_RAW_DPI, &dpiX, &dpiY)))
			oic::System::log()->fatal(std::system_error(hr, std::system_category()).what());

		Vec2f32 screenSize = { f32(rect->right - rect->left), f32(rect->bottom - rect->top) };
		Vec2f32 sizeInMeters = screenSize / Vec2f32(f32(dpiX), f32(dpiY)) * 0.0254f;

		//Convert samples to rotated space

		if (orientation >= Orientation::LANDSCAPE_FLIPPED) {
			samples[0].x *= -1;
			samples[2].x *= -1;
		}

		if (orientation == Orientation::PORTRAIT || orientation == Orientation::PORTRAIT_FLIPPED) {
			samples[0] = samples[0].swap();
			samples[2] = samples[2].swap();
		}

		//Add monitor with all specs like position, dimension

		data->monitors->push_back(
			Monitor {
				Vec2i32(rect->left, rect->top),
				orientation,
				refreshRate,

				Vec2i32(rect->right, rect->bottom),
				gamma,
				contrast,

				samples[0],
				samples[1],

				samples[2],
				sizeInMeters
			}
		);

		return true;
	}

	inline bool inView(const Vec2i32 &min, const Vec2i32 &max, const Monitor &other) {

		return
			min.x < other.max.x && max.x > other.min.x &&
			min.y < other.max.y && max.y > other.min.y;
	}

	List<Monitor> getMonitorsFromWindow(WWindow *w) {

		auto &screens = oic::System::viewportManager()->getScreens();

		List<Monitor> monitors;

		Vec2i32 min = w->info->offset;
		Vec2i32 max = min + w->info->size.cast<Vec2i32>();

		for (auto &mon : screens)
			if (inView(min, max, mon))
				monitors.push_back(mon);

		return monitors;
	}

	WViewportManager::WViewportManager(): instance(GetModuleHandleA(NULL)) {

		//Inititialize 

		ForeachMonitorData fmonitorData{};
		fmonitorData.monitors = &screens;

		HRESULT hr =
			DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_ISOLATED,
				__uuidof(IDWriteFactory),
				(IUnknown**)&fmonitorData.factory
			);

		if (SUCCEEDED(hr))
			EnumDisplayMonitors(nullptr, nullptr, (MONITORENUMPROC) foreachMonitor, (LPARAM) &fmonitorData);

		fmonitorData.factory->Release();
	
		//TODO: Register callback for when monitors change
		//RegisterDeviceNotificationA
	}
	
	WViewportManager::~WViewportManager() {

		for (ViewportInfo *info : *this)
			del(info);
	}

	WWindow::WWindow(ViewportInfo *info, HINSTANCE instance): info(info), hwnd(), running(false) {

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

		Vec2u32 maxSize = { u32(GetSystemMetrics(SM_CXSCREEN)), u32(GetSystemMetrics(SM_CYSCREEN)) };

		for (usz i = 0; i < 2; ++i)
			if (!info->size[i] || info->size[i] >= maxSize[i])
				info->size[i] = maxSize[i];

		hwnd = CreateWindowExA(
			WS_EX_APPWINDOW, wc.lpszClassName, wc.lpszClassName, style,
			info->offset.x, info->offset.y, info->size.x, info->size.y,
			NULL, NULL, instance, NULL
		);

		RECT r{};
		GetClientRect(hwnd, &r);
		info->size = { u32(r.right - r.left), u32(r.bottom - r.top) };

		GetWindowRect(hwnd, &r);
		info->offset = { r.left, r.top };

		if(!hwnd) {
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

		w->info->hint = ViewportInfo::Hint(ViewportInfo::IS_FOCUSSED | w->info->hint);

		if (w->info->vinterface) {
			w->info->vinterface->init(w->info);
			w->info->monitors = getMonitorsFromWindow(w);
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

			case WM_SETFOCUS:

				if (auto *ptr = (WWindow*) GetWindowLongPtrA(hwnd, 0))
					ptr->info->hint = ViewportInfo::Hint(ptr->info->hint | ViewportInfo::IS_FOCUSSED);

				break;

			case WM_KILLFOCUS:

				if (auto *ptr = (WWindow*) GetWindowLongPtrA(hwnd, 0))
					ptr->info->hint = ViewportInfo::Hint(ptr->info->hint & ~ViewportInfo::IS_FOCUSSED);

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
					if (auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0)) {

						auto *vinterface = ptr->info->vinterface;

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

								if(pressed != isKeyDown)
									if (vinterface) {

										if (isKeyDown)
											vinterface->onInputActivate(ptr->info, dvc, InputHandle(keyCode));
										else 
											vinterface->onInputDeactivate(ptr->info, dvc, InputHandle(keyCode));

										vinterface->onInputUpdate(ptr->info, dvc, InputHandle(keyCode), isKeyDown);
									};
							}

							return 0;

						} else {

							auto &mouseDat = data->data.mouse;

							for (usz i = 0; i < 5; ++i) {

								if (mouseDat.usButtonFlags & (1 << (i << 1))) {

									dvc->setPreviousState(ButtonHandle(i), dvc->getPreviousState(ButtonHandle(i)));
									dvc->setState(ButtonHandle(i), true);

									if (vinterface) {
										vinterface->onInputActivate(ptr->info, dvc, InputDevice::Handle(i));
										vinterface->onInputUpdate(ptr->info, dvc, InputHandle(i), true);
									}
								}

								else if (mouseDat.usButtonFlags & (2 << (i << 1))) {

									dvc->setPreviousState(ButtonHandle(i), true);
									dvc->setState(ButtonHandle(i), false);

									if (vinterface) {
										vinterface->onInputDeactivate(ptr->info, dvc, InputDevice::Handle(i));
										vinterface->onInputUpdate(ptr->info, dvc, InputHandle(i), false);
									}
								}

							}

							if (mouseDat.usButtonFlags & RI_MOUSE_WHEEL) {

								f64 delta = i16(mouseDat.usButtonData);
								dvc->setAxis(MouseAxis::Axis_wheel, delta);

								if (vinterface)
									vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_wheel) + MouseButton::count, delta != 0);
							}

							if (mouseDat.usFlags & MOUSE_MOVE_ABSOLUTE) {

								f64 x = f64(mouseDat.lLastX) - rect.left, y = f64(mouseDat.lLastY) - rect.top;

								dvc->setAxis(MouseAxis::Axis_delta_x, dvc->getCurrentAxis(MouseAxis::Axis_x) - x);
								dvc->setAxis(MouseAxis::Axis_delta_y, dvc->getCurrentAxis(MouseAxis::Axis_y) - y);
								dvc->setAxis(MouseAxis::Axis_x, x);
								dvc->setAxis(MouseAxis::Axis_y, y);

								vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_delta_x) + MouseButton::count, mouseDat.lLastX != 0);
								vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_delta_y) + MouseButton::count, mouseDat.lLastY != 0);
								vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_x) + MouseButton::count, false);
								vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_y) + MouseButton::count, false);

							} else {

								dvc->setAxis(MouseAxis::Axis_delta_x, mouseDat.lLastX);
								dvc->setAxis(MouseAxis::Axis_delta_y, mouseDat.lLastY);

								if (vinterface) {
									vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_delta_x) + MouseButton::count, mouseDat.lLastX != 0);
									vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_delta_y) + MouseButton::count, mouseDat.lLastY != 0);
								}
							}
						}

					}

				return DefRawInputProc(&data, 1, sizeof(*data));
			}

			case WM_MOUSEMOVE:

				if (auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0)) {

					for (auto device : ptr->devices) {

						auto *dvc = device.second;

						if (dvc->getType() != InputDevice::Type::MOUSE)
							continue;

						if (ptr->info->hasHint(ViewportInfo::CAPTURE_CURSOR))
							return false;

						int x = GET_X_LPARAM(lParam); 
						int y = GET_Y_LPARAM(lParam);

						dvc->setAxis(MouseAxis::Axis_x, f64(x));
						dvc->setAxis(MouseAxis::Axis_y, f64(y));

						if (auto *vinterface = ptr->info->vinterface) {
							vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_x) + MouseButton::count, false);
							vinterface->onInputUpdate(ptr->info, dvc, InputHandle(MouseAxis::Axis_y) + MouseButton::count, false);
						}
					}

				}

				break;

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
							dvc->setAxis(MouseAxis::Axis_x, point.x);
							dvc->setAxis(MouseAxis::Axis_y, point.y);
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

			case WM_PAINT: {

				auto *ptr = (WWindow *) GetWindowLongPtrA(hwnd, 0);

				if (ptr && !ptr->info->hasHint(ViewportInfo::IS_MINIMIZED)) {

					//Update interface

					ns now = oic::Timer::now();
					auto *info = ptr->info;

					if (info->vinterface) {
						f64 dt = ptr->last ? (now - ptr->last) / 1'000'000'000.0 : 0;
						info->vinterface->update(info, dt);
					}

					ptr->last = oic::Timer::now();

					//Update input

					for (auto dvc : info->devices) {

						for (ButtonHandle i = 0, j = ButtonHandle(dvc->getButtonCount()); i < j; ++i)
							if (dvc->getState(i) == 0x2 /* released */)
								dvc->setPreviousState(i, false);
							else if (dvc->getState(i) == 0x1 /* pressed */)
								dvc->setPreviousState(i, true);

						for (AxisHandle i = 0, j = dvc->getAxisCount(); i < j; ++i)
							dvc->setPreviousAxis(i, dvc->getCurrentAxis(i));
					}

					if (ptr->running && (info->size.neq(0)).all() && info->vinterface)
						info->vinterface->render(info);
				}

				return NULL;
			}

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

				RECT r;
				GetClientRect(hwnd, &r);
				Vec2u32 newSize = { u32(r.right - r.left), u32(r.bottom - r.top) };

				if (!ptr) break;

				if (wParam == SIZE_MINIMIZED) 
					ptr->info->hint = ViewportInfo::Hint(ptr->info->hint | ViewportInfo::IS_MINIMIZED);
				else
					ptr->info->hint = ViewportInfo::Hint(ptr->info->hint & ~ViewportInfo::IS_MINIMIZED);

				if (!newSize.all() || newSize == ptr->info->size)
					break;

				ptr->info->size = newSize;
				ptr->info->monitors = getMonitorsFromWindow(ptr);

				if (ptr->info->vinterface)
						ptr->info->vinterface->resize(ptr->info, ptr->info->size);

				break;
			}

			case WM_MOVE: {
					
				auto *ptr = (WWindow*)GetWindowLongPtrA(hwnd, 0);

				if (!ptr)
					break;

				RECT r;
				GetWindowRect(hwnd, &r);

				ptr->info->offset = { r.left, r.top };
				ptr->info->monitors = getMonitorsFromWindow(ptr);
				break;
			}
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void WViewportManager::redraw(const ViewportInfo *info) {
		RedrawWindow(hwnd[info->id]->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}

}