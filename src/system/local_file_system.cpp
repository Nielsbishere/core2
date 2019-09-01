#include "system/local_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "error/ocore.hpp"

//64-bit types for Unix

#if INTPTR_MAX == INT64_MAX && !defined(_WIN32)
	#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>

//Platform wrappers

#ifdef _WIN32
	#include <direct.h>
	#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#ifdef _WIN64
	#define fseeko _fseeki64
	#define stat _stat64
#elif _WIN32
	#define fseeko _fseek
#else
	#define _mkdir(x) mkdir(x, 0600)
	#define fopen_s(res, ...) *(res) = fopen(__VA_ARGS__)
#endif

//Local file system implementation

namespace oic {

	LocalFileSystem::LocalFileSystem(String localPath): 
		FileSystem(Array<FileAccess, 2>{ FileAccess::READ, FileAccess::READ_WRITE }), 
		localPath(localPath) {}

	const String &LocalFileSystem::getLocalPath() const {
		return localPath;
	}

	bool LocalFileSystem::read(const FileInfo &file, Buffer &buffer, usz size, usz offset) const {

		if (!file.isLocal())
			return readVirtual(file, buffer, size, offset);

		if (file.isFolder) {
			System::log()->fatal(errors::fs::illegal);
			return false;
		}

		if (!file.hasAccess(FileAccess::READ)) {
			System::log()->fatal(errors::fs::notPermitted);
			return false;
		}
		
		FILE *f{};
		if(fopen_s(&f, file.path.c_str(), "r") != 0 || !f) {
			System::log()->fatal(errors::fs::nonExistent);
			return false;
		}

		if (!size)
			size = file.fileSize - (offset >= file.fileSize ? 0 : offset);

		if (offset + size > file.fileSize) {
			System::log()->fatal(errors::fs::outOfBounds);
			fclose(f);
			return false;
		}

		buffer.resize(size);

		fread(buffer.data() + offset, 1, size, f);
		fclose(f);
		return true;
	}

	bool LocalFileSystem::write(FileInfo &file, const Buffer &buffer, usz size, usz bufferOffset, usz fileOffset) {

		if (!file.isLocal())
			return writeVirtual(file, buffer, size, bufferOffset, fileOffset);

		if (file.isFolder) {
			System::log()->fatal(errors::fs::illegal);
			return false;
		}

		if (!file.hasAccess(FileAccess::WRITE)) {
			System::log()->fatal(errors::fs::notPermitted);
			return false;
		}

		FILE *f{};
		if(fopen_s(&f, file.path.c_str(), fileOffset == usz_MAX ? "a" : "w") != 0 || !f) {
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
		upd(file.path);
		return true;
	}

	bool LocalFileSystem::make(FileInfo &file) {

		if (!file.isLocal()) {
			System::log()->fatal(errors::fs::notSupported);
			return false;
		}

		if (file.isFolder) {

			if (_mkdir(file.path.c_str()) != 0) {
				System::log()->fatal(errors::fs::illegal);
				return false;
			}

		} else {

			FILE *f{};
			if (fopen_s(&f, file.path.c_str(), "w") != 0 || !f) {
				System::log()->fatal(errors::fs::nonExistent);
				return false;
			}

		}

		return true;
	}

	void LocalFileSystem::onFileChange(FileInfo &file, FileChange change) {

		if (change == FileChange::REM)
			return;

		if (!file.isLocal()) {
			onVirtualFileChange(file, change);
			return;
		}

		struct stat st;
		stat(file.path.c_str(), &st);

		file.modificationTime = st.st_mtime;
		file.fileSize = st.st_size;

	}

	void LocalFileSystem::initFile(FileInfo &file) {

		if (!file.isLocal())
			return;

		struct stat st;
		stat(file.path.c_str(), &st);

		file.modificationTime = st.st_mtime;

		if(file.id != 0)
			file.access = get(file.parent, file.isLocal()).access;

		file.isFolder = !S_ISREG(st.st_mode);
		file.fileSize = st.st_size * usz(!file.isFolder);

	}

}