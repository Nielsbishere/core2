#pragma once
#include "types/types.hpp"
#include "types/vec.hpp"
#include <mutex>

namespace oic {

	class ViewportInterface;

	class InputDevice;

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

		//Used for when communicating data to sync
		std::mutex fence;

		ViewportInfo(
			const String &name, Vec2i32 offset, Vec2u32 size,
			u32 layer, ViewportInterface *vinterface, Hint hint = NONE)
			:
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

	protected:

		ViewportInfo *find(const String &name);
		bool destroyTopLevel(const ViewportInfo *info);		//Only destroys top level data

		virtual void add(ViewportInfo *info) = 0;
		virtual void del(const ViewportInfo *info) = 0;

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