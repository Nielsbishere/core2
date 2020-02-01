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

		File *open(const FileInfo &info, ns maxTimeout, ns retryTimeout) final override;

	protected:

		//!Make or delete files
		void onFileChange(const FileInfo &file, FileChange change) final override;

		//!Create a file/folder in the physical directory
		bool makeLocal(const String &path, bool isFolder) final override;

		//!Remove a file/folder in the physical directory
		bool delLocal(const String &path) final override;

		//!Open virtual file
		virtual File *openVirtual(const FileInfo &file) = 0;


		const FileInfo local(const String &path) const final override;
		bool hasLocal(const String &path) const final override;
		bool hasLocalRegion(const String &path, FileSize size, FileSize offset) const final override;

		//!Called on virtual file system change
		//@param[inout] FileInfo &file
		//@param[in] FileChange change
		virtual void onVirtualFileChange(const FileInfo &, FileChange) { }

	private:

		String localPath;

	};

}