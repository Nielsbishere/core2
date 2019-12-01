#include "system/viewport_manager.hpp"
#include "system/viewport_interface.hpp"
#include "input/input_device.hpp"

namespace oic {

	bool ViewportManager::destroy(const ViewportInfo *info) {

		auto it = find(info);

		if (it == viewports.end())
			return false;

		if (info->vinterface)
			info->vinterface->release(info);

		for (u32 i = (*it)->id + 1, end = u32(viewports.size()); i < end; ++i) {
			--viewports[i]->id;
			--idMap.find(viewports[i]->name)->second;
		}

		for (auto *dev : info->devices)
			delete dev;

		(*it)->devices.clear();

		idMap.erase(info->name);
		viewports.erase(it);
		del(*it);
		delete info;
		return true;
	}

	bool ViewportManager::destroyTopLevel(const ViewportInfo *info) {

		auto it = find(info);

		if (it == viewports.end())
			return false;

		if (info->vinterface)
			info->vinterface->release(info);

		for (u32 i = (*it)->id + 1, end = u32(viewports.size()); i < end; ++i) {
			--viewports[i]->id;
			--idMap.find(viewports[i]->name)->second;
		}

		for (auto *dev : info->devices)
			delete dev;

		(*it)->devices.clear();

		idMap.erase(info->name);
		viewports.erase(it);
		delete info;
		return true;
	}

	bool ViewportManager::create(const ViewportInfo &info) {

		if (find(info.name) != nullptr)
			return false;

		usz lid = viewports.size();

		if (lid >= u32_MAX)
			return false;

		u32 id = u32(lid);

		ViewportInfo *vpi = new ViewportInfo(info.name, info.offset, info.size, info.layer, info.vinterface, info.hint);
		viewports.push_back(vpi);

		viewports[id]->id = id;
		idMap[info.name] = id;
		add(vpi);
		return true;
	}

	void ViewportManager::clear() {

		for (const ViewportInfo *viewport : viewports) {
			del(viewport);
			delete viewport;
		}

		viewports.clear();
		idMap.clear();
	}

	ViewportInfo *ViewportManager::find(const String &name) {

		auto it = idMap.find(name);

		if (it == idMap.end())
			return nullptr;

		return viewports[it->second];
	}

	void ViewportManager::waitSignal(const ViewportInfo *info) { 
		((ViewportInfo*)info)->fence.lock(); 
	}

	void ViewportManager::resetSignal(const ViewportInfo *info) {
		 ((ViewportInfo*)info)->fence.unlock();
	}

	List<ViewportInfo*>::const_iterator ViewportManager::begin() const {
		return viewports.begin();
	}

	List<ViewportInfo*>::const_iterator ViewportManager::end() const {
		return viewports.end();
	}

}