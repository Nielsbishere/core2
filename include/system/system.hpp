#pragma once

namespace oic {

    class LocalFileSystem;
    class Allocator;
    class ViewportManager;
    class Log;

    //!Platform specific wrapper holding the file system, allocator, window manager and log
    //There may only be one instance of this object that has to be garbage collected
	class System {

	public:

		static inline LocalFileSystem *files() { return system->files_; }
		static inline Allocator *allocator() { return system->allocator_; }
		static inline ViewportManager *viewportManager() { return system->viewportManager_; }
		static inline Log *log() { return system->log_; }

		//!Used to replace the native Log callback with a custom one
		//For example, logging to file, to UI, etc.
		//@param[in] log The log class that handles the print callbacks, nullptr to reset it to native
		//@note The System class handles cleanup of the Log* passed
		static void setCustomLogCallback(Log *log);

		//!Terminate the program; will stop System from updating
		static void terminate();

		//!Wait until the viewport manager is done and everything can be cleaned up
		static void wait();

	protected:

		System(LocalFileSystem *files_, Allocator *allocator_, ViewportManager *viewportManager_, Log *nativeLog);
		virtual ~System();

		System(const System &) = delete;
		System(System &&) = delete;
		System &operator=(const System &) = delete;
		System &operator=(System &&) = delete;

		//Values setup by the child class

		LocalFileSystem *files_{};
		Allocator *allocator_{};
		ViewportManager *viewportManager_{};
		Log *log_{}, *nativeLog{};

		bool isActive = true;

		//System class

        static System *system;

    };

}