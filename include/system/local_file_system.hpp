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
		using FileSystem::open;

		//!Gets the path to the ./ directory
		//Only useable as a hint to the end user
		const String &getLocalPath() const;

		File *open(FileInfo &info) final override;

	protected:

		//!Update the file size and modification time of the local folder
		void onFileChange(FileInfo &path, FileChange change) final override;

		//!Create a folder in the physical directory
		bool make(FileInfo &file) final override;

		//!Setup the data of the file
		//requires path and file location variables to be set
		void initFile(FileInfo &file);

		//!Open virtual file
		virtual File *openVirtual(FileInfo &file) = 0;

		//!Called on virtual file system change
		//@param[inout] FileInfo &file
		//@param[in] FileChange change
		virtual void onVirtualFileChange(FileInfo &, FileChange) { }

		//!Initialize the watcher that updates the local file system
		//Should be called from child class
		virtual void initFileWatcher() = 0;

	private:

		String localPath;

	};

}