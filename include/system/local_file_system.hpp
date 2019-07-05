#pragma once
#include "types/types.hpp"
#include "system/file_system.hpp"

namespace oic {

	//!Subclass for file systems that are linked to a directory
	class LocalFileSystem : public FileSystem {

	public:

		LocalFileSystem(String localPath);

		using FileSystem::write;
		using FileSystem::read;

		//!Gets the path to the ./ directory
		//Only useable as a hint to the end user
		const String &getLocalPath() const;

		//!Read the local file using the C File API
		bool read(const FileInfo &file, Buffer &buffer, usz size = 0, usz offset = 0) const final override;

		//!Write to the local file using the C File API
		bool write(FileInfo &file, const Buffer &buffer, usz size = 0, usz bufferOffset = 0, usz fileOffset = 0) final override;

	protected:

		//!Update the file size and modification time of the local folder
		void onFileChange(FileInfo &path, bool remove) final override;

		//!Create a folder in the physical directory
		void mkdir(FileInfo &file) final override;

		//!Setup the data of the file
		//requires path and file location variables to be set
		void initFile(FileInfo &file);

		//!Read the virtual file
		virtual bool readVirtual(const FileInfo &file, Buffer &buffer, usz size, usz offset) const = 0;

		//!Write to the virtual file
		//Since most virtual file systems are read only, this is optional
		//@param[inout] FileInfo &file
		//@param[in] const Buffer &toWrite
		//@param[in] usz size
		//@param[in] usz bufferOffset
		//@param[in] usz fileOffset
		//@return false if the virtual file can't be written or writing isn't supported
		virtual bool writeVirtual(FileInfo &, const Buffer &, usz, usz, usz) { return false; }

		//!Called on virtual file system change
		//@param[inout] FileInfo &file
		//@param[in] bool isRemoved
		virtual void onVirtualFileChange(FileInfo &, bool) { }

		//!Initialize the watcher that updates the local file system
		//Should be called from child class
		virtual void initFileWatcher() = 0;

	private:

		String localPath;

	};

}