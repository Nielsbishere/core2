#include <stdexcept>
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"

namespace oic {

	System::System(LocalFileSystem *files_, Allocator *allocator_, ViewportManager *viewportManager_, Log *nativeLog):
		files_(files_), allocator_(allocator_), viewportManager_(viewportManager_), nativeLog(nativeLog), log_(nativeLog) {

		if (system)
			nativeLog->fatal(errors::sys::alreadyExists);

		system = this;
	}

	System::~System() {

		if (log_ != nativeLog)
			delete log_;

		system = nullptr;
	}

	void System::setCustomLogCallback(Log *log) {

		if (system->log_ != system->nativeLog)
			delete system->log_;

		system->log_ = log ? log : system->nativeLog;
	}

	System *System::system = nullptr;

	void System::terminate() {
		system->isActive = false;
	}

	void System::wait() {
		while (system->isActive) {
			/*system->viewportManager_->update();
			system->files_->update();*/
		}
	}

	void System::begin() {
		system->mutex.lock();
	}

	void System::end() {
		system->mutex.unlock();
	}


}