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

		static inline LocalFileSystem *files();
		static inline Allocator *allocator();
		static inline ViewportManager *viewportManager();
		static inline Log *log();

	protected:

		System();
		virtual ~System();

		System(const System &) = delete;
		System(System &&) = delete;
		System &operator=(const System &) = delete;
		System &operator=(System &&) = delete;

		virtual LocalFileSystem *getFiles() const = 0;
		virtual Allocator *getAllocator() const = 0;
		virtual ViewportManager *getViewportManager() const = 0;
		virtual Log *getLog() const = 0;

        static const System *system;

    };

}