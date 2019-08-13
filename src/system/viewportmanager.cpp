#include "system/viewport_manager.hpp"

namespace oic {

	bool ViewportManager::destroy(const ViewportInfo &info) {

		auto it = find(info);

		if (it == viewports.end())
			return false;

		for (u32 i = it->id + 1, end = u32(viewports.size()); i < end; ++i) {
			--viewports[i].id;
			--idMap.find(viewports[i].name)->second;
		}

		del(*it);
		idMap.erase(info.name);
		viewports.erase(it);
		return true;
	}

	bool ViewportManager::create(const ViewportInfo &info) {

		if (find(info.name) != nullptr)
			return false;

		usz lid = viewports.size();

		if (lid >= u32_MAX)
			return false;

		u32 id = u32(lid);

		viewports.push_back(info);
		viewports[id].id = id;
		idMap[info.name] = id;
		add(viewports[id]);
		return true;
	}

	void ViewportManager::clear() {

		for (const ViewportInfo &viewport : viewports)
			del(viewport);

		viewports.clear();
		idMap.clear();
	}

}