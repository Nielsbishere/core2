#include <stdexcept>
#include "system/system.hpp"

namespace oic {

	System::System() { 

		if(system)
			throw std::runtime_error("There can only be one system");

		system = this;
	}

	System::~System() {
		system = nullptr;
	}
    
    const System *System::system = nullptr;

	LocalFileSystem *System::files() {
		return system->getFiles();
	}

	Allocator *System::allocator() {
		return system->getAllocator();
	}

	ViewportManager *System::viewportManager() {
		return system->getViewportManager();
	}

	Log *System::log() {
		return system->getLog();
	}


}