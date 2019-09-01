#include <stdexcept>
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"
#include "system/viewport_manager.hpp"

namespace oic {

	System::System(LocalFileSystem *files_, Allocator *allocator_, ViewportManager *viewportManager_, Log *nativeLog):
		files_(files_), allocator_(allocator_), viewportManager_(viewportManager_), nativeLog(nativeLog), log_(nativeLog) {

		if (!system)
			system = this;
	}

	System::~System() {

		viewportManager_->clear();

		if (log_ != nativeLog)
			delete log_;

		if(system == this)
			system = nullptr;
		
	}

	void System::setCustomLogCallback(Log *log) {

		if (system->log_ != system->nativeLog)
			delete system->log_;

		system->log_ = log ? log : system->nativeLog;
	}

	void System::begin() {
		system->mutex.lock();
	}

	void System::end() {
		system->mutex.unlock();
	}

	void System::wait(u64 time) {
		system->sleep(time);
	}


}