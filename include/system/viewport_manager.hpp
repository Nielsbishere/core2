#pragma once
#include "types/types.hpp"

namespace oic {

	struct ViewportInfo {

		enum Hint : u8 {
			NONE,
			FULL_SCREEN = 1,
			NO_MENU = 2,
			NOT_RESIZABLE = 4,
			NOT_MINIMIZABLE = 8
		};

		//The name of this layer
		String name;

		//Position hints of the current viewport
		Vec2i offset;
		Vec2u size;
		u32 layer;
		
		//The id of this viewport
		u32 id;

		//Hints of how this viewport should be created
		//These are only HINTS, the underlying implementation can decide if its appropriate
		Hint hint;

		ViewportInfo(const String &name, Vec2i offset, Vec2u size, u32 layer, Hint hint = NONE):
			name(name), offset(offset), size(size), layer(layer), id(), hint(hint) {}

		inline bool hasHint(Hint h) const {
			return hint & h;
		}

		inline bool operator==(const ViewportInfo &info) const {
			return id == info.id && name == info.name;
		}

	};

	class ViewportManager {

	public:

		ViewportManager() = default;
		virtual ~ViewportManager() = default;
		
		ViewportManager(const ViewportManager&) = delete;
		ViewportManager(ViewportManager&&) = delete;
		ViewportManager &operator=(const ViewportManager&) = delete;
		ViewportManager &operator=(ViewportManager&&) = delete;

		inline const ViewportInfo *find(const String &name) const {

			auto it = idMap.find(name);

			if (it == idMap.end())
				return nullptr;

			return viewports.data() + it->second;
		}

		inline const ViewportInfo &operator[](usz i) const {
			return viewports[i];
		}

		inline auto find(const ViewportInfo &info) {
			return std::find(viewports.begin(), viewports.end(), info);
		}

		inline bool contains(const ViewportInfo &info) {
			return find(info) != viewports.end();
		}

		bool destroy(const ViewportInfo &info);
		bool create(const ViewportInfo &info);

		void clear();

	protected:

		virtual void add(ViewportInfo &info) = 0;
		virtual void del(const ViewportInfo &info) = 0;

	private:

		List<ViewportInfo> viewports;
		HashMap<String, usz> idMap;

	};

}