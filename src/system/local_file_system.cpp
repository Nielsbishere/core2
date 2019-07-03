#include "system/local_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"

//64-bit types for Unix

#if INTPTR_MAX == INT64_MAX
	#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>

//Platform wrappers

#ifdef _WIN64
	#include <direct.h>
	#define fseeko _fseeki64
	#define stat _stat64
#else
	#define _mkdir(x) mkdir(x, 0600)
#endif

//Local file system implementation

namespace oic {

	LocalFileSystem::LocalFileSystem(): FileSystem(true) {
		initFileWatcher();
	}

	bool LocalFileSystem::read(const FileInfo &file, Buffer &buffer, usz size, usz offset) const {

		if (file.isVirtual())
			return readVirtual(file, buffer, size, offset);
		
		FILE *f = fopen(file.path.c_str(), "r");

		if (!f) {
			System::log()->fatal(errors::fs::nonExistent);
			return false;
		}

		if (!size)
			size = file.fileSize - (offset >= file.fileSize ? 0 : offset);

		if (offset + size > file.fileSize) {
			System::log()->fatal(errors::fs::outOfBounds);
			return false;
		}

		buffer.resize(size);

		fread(buffer.data() + offset, 1, size, f);
		fclose(f);
		return true;
	}

	bool LocalFileSystem::write(FileInfo &file, const Buffer &buffer, usz size, usz bufferOffset, usz fileOffset) {

		if (file.isVirtual())
			return writeVirtual(file, buffer, size, bufferOffset, fileOffset);

		FILE *f = fopen(file.path.c_str(), fileOffset == usz_MAX ? "a" : "w");

		if (!f) {
			System::log()->fatal(errors::fs::nonExistent);
			return false;
		}

		if (bufferOffset + size > buffer.size()) {
			System::log()->fatal(errors::fs::outOfBounds);
			return false;
		}

		if (fileOffset == usz_MAX)
			file.fileSize += size;

		else if (fileOffset > file.fileSize) {
			System::log()->fatal(errors::fs::outOfBounds);
			return false;
		}
		else if(fileOffset + size > file.fileSize)
			file.fileSize = fileOffset + size;

		if(fileOffset != 0 && fileOffset != usz_MAX)
			fseeko(f, fileOffset, 0);

		fwrite(buffer.data() + bufferOffset, 1, size, f);
		fclose(f);
		notifyFileChange(file.path, false);
		return true;
	}

	void LocalFileSystem::mkdir(FileInfo &file) {

		if (_mkdir(file.path.c_str()) != 0)
			System::log()->fatal(errors::fs::notSupported);
	}

	void LocalFileSystem::onFileChange(FileInfo &file, bool remove) {

		if (!remove)
			return;

		if (file.isVirtual()) {
			onVirtualFileChange(file, remove);
			return;
		}

		struct stat st;
		stat(file.path.c_str(), &st);

		file.modificationTime = st.st_mtime;
		file.fileSize = st.st_size;

	}
}