#pragma once
#include "types/types.hpp"
#include "types/vec.hpp"
#include <mutex>
#include <cstring>
#include <algorithm>

namespace oic {

	class ViewportInterface;

	class InputDevice;

	enum class Orientation : u32 {
		LANDSCAPE = 0,
		PORTRAIT = 90,
		LANDSCAPE_FLIPPED = 180,
		PORTRAIT_FLIPPED = 270
	};

	struct Monitor {

		Vec2i32 min;				//Start position in virtual screen space
		Orientation orientation;	//Physical orientation
		i32 refreshRate;			//Hz (frames / second)

		Vec2i32 max;				//End position of the monitor in virtual screen space
		f32 gamma;
		f32 contrast;

		Vec2f32 sampleR, sampleG;	//Where to take the R/G samples from (transformed by rotation)

		Vec2f32 sampleB;			//Where to take B sample from (transformed by rotation)
		Vec2f32 sizeInMeters;		//Physical size of the monitor

		inline Vec2i32 size() const { return max - min; }

		inline bool operator==(const Monitor &other) const { 
			return std::memcmp(this, &other, sizeof(other)) == 0; 
		}

		inline bool operator!=(const Monitor &other) const {
			return !operator==(other);
		}

	};

	struct ViewportInfo {

		enum Hint : u8 {
			NONE,
			FULL_SCREEN,
			NO_MENU,
			NOT_RESIZABLE = 4,
			NOT_MINIMIZABLE = 8,
			HANDLE_INPUT = 16
		};

		//The name of this layer
		String name;

		//Position hints of the current viewport
		Vec2i32 offset;
		Vec2u32 size;
		u32 layer;
		
		//The id of this viewport
		u32 id;

		//Hints of how this viewport should be created
		//These are only HINTS, the underlying implementation can decide if its appropriate
		Hint hint;
		
		//Callback for viewport functions
		ViewportInterface *vinterface;

		//Devices
		List<InputDevice*> devices;

		//Monitors that this device is active on
		List<Monitor> monitors;

		//Used for when communicating data to sync
		std::mutex fence;

		ViewportInfo(
			const String &name, const Vec2i32 &offset, const Vec2u32 &size,
			u32 layer, ViewportInterface *vinterface, Hint hint = NONE
		) :
			name(name), offset(offset), size(size), layer(layer),
			id(), hint(hint), vinterface(vinterface) {}

		ViewportInfo(ViewportInfo&&) = delete;
		ViewportInfo &operator=(ViewportInfo&&) = delete;
		ViewportInfo(const ViewportInfo&) = delete;
		ViewportInfo &operator=(const ViewportInfo&) = delete;

		inline bool hasHint(Hint h) const {
			return hint & h;
		}

		inline bool operator==(const ViewportInfo &info) const {
			return id == info.id && name == info.name;
		}

	};

	//Requires implementation to handle destruction of viewports in destructor
	class ViewportManager {

	public:

		ViewportManager() = default;
		virtual ~ViewportManager() = default;
		
		ViewportManager(const ViewportManager&) = delete;
		ViewportManager(ViewportManager&&) = delete;
		ViewportManager &operator=(const ViewportManager&) = delete;
		ViewportManager &operator=(ViewportManager&&) = delete;

		inline const ViewportInfo *find(const String &name) const;
		inline const ViewportInfo *operator[](usz i) const;
		inline auto find(const ViewportInfo *info);
		inline bool contains(const ViewportInfo *info) const;
		inline usz size() const;

		bool destroy(const ViewportInfo *info);
		bool create(const ViewportInfo &info);

		void clear();

		//Called to wait for the viewport to be available (unused by the I/O loop)
		void waitSignal(const ViewportInfo *info);

		//Called to make the viewport available (so it can be used by the I/O loop)
		void resetSignal(const ViewportInfo *info);

		//Requests a redraw of the screen
		virtual void redraw(const ViewportInfo *info) = 0;

		List<ViewportInfo*>::const_iterator begin() const;
		List<ViewportInfo*>::const_iterator end() const;

		inline const Monitor &getScreen(usz i) const { return screens[i]; }
		inline usz getScreenCount() const { return screens.size(); }
		inline const auto &getScreens() const { return screens; }

	protected:

		ViewportInfo *find(const String &name);
		bool destroyTopLevel(const ViewportInfo *info);		//Only destroys top level data

		virtual void add(ViewportInfo *info) = 0;
		virtual void del(const ViewportInfo *info) = 0;

		List<Monitor> screens;

	private:

		List<ViewportInfo*> viewports;
		HashMap<String, usz> idMap;

	};

	
	inline const ViewportInfo *ViewportManager::find(const String &name) const { 
		return ((ViewportManager*)this)->find(name); 
	}

	inline const ViewportInfo *ViewportManager::operator[](usz i) const {
		return viewports[i];
	}

	inline auto ViewportManager::find(const ViewportInfo *info) {
		return std::find(viewports.begin(), viewports.end(), info);
	}

	inline bool ViewportManager::contains(const ViewportInfo *info) const {
		return std::find(viewports.begin(), viewports.end(), info) != viewports.end();
	}

	inline usz ViewportManager::size() const { 
		return viewports.size();
	}

}