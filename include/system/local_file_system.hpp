#pragma once
#include "types/types.hpp"
#include "system/file_system.hpp"

namespace oic {

	//!Subclass for file systems that are linked to a directory
	class LocalFileSystem : public FileSystem {

	public:

		LocalFileSystem();

		using FileSystem::read;
		using FileSystem::write;

		virtual String getLocalPath() const = 0;

		bool read(const FileInfo &file, Buffer &buffer, usz size = 0, usz offset = 0) const final override;
		bool write(FileInfo &file, const Buffer &buffer, usz size = 0, usz bufferOffset = 0, usz fileOffset = 0) final override;

		void mkdir(FileInfo &file) final override;

	protected:

		void onFileChange(FileInfo &path, bool remove) final override;

		virtual bool readVirtual(const FileInfo &file, Buffer &buffer, usz size, usz offset) const = 0;
		virtual bool writeVirtual(FileInfo &, const Buffer &, usz, usz, usz) { return false; }
		virtual void onVirtualFileChange(FileInfo &, bool) { }

		virtual void initFileWatcher() = 0;


	};

}