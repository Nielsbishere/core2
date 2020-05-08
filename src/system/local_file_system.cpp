#include "system/local_file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

//64-bit types for Unix

#if INTPTR_MAX == INT64_MAX && !defined(_WIN32)
	#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>

//Platform wrappers

#ifdef _WIN32
	#include <direct.h>
	#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#else
	#include <sys/stat.h>
#endif

#ifdef _WIN64
	#define fseeko _fseeki64
	#define stat _stat64
#elif _WIN32
	#define fseeko _fseek
#else
	#define _mkdir(x) mkdir(x, 0600)
	inline bool fopen_s(FILE **f, const c8 *path, const c8 *perm) { return !(*f = fopen(path, perm)); }
#endif

//Local file system implementation

namespace oic {

	class CFile : public File {

	private:

		FILE *file{};

		virtual ~CFile() { 
			fclose(file); 
			file = nullptr;
		}

	public:

		CFile(FileInfo &f): File(f) {
		
			if (f.hasAccess(FileAccess::WRITE)) {

				if (fopen_s(&file, f.path.c_str(), "a+b"))
					System::log()->fatal("File doesn't exist");

				else isOpen = true;

			} else if (f.hasAccess(FileAccess::READ)) {

				if (fopen_s(&file, f.path.c_str(), "rb"))
					System::log()->fatal("File doesn't exist");

				else isOpen = true;
			}
		
		}

		bool read(void *v, FileSize size, FileSize offset) const final override {

			if (!file) {
				System::log()->fatal("Read access isn't permitted");
				return false;
			}

			if (offset + size > f.fileSize) {
				System::log()->fatal("File read is out of bounds");
				return false;
			}

			fseeko(file, offset, 0);
			return fread(v, 1, size, file);
		}

		bool write(const void *v, FileSize size, FileSize offset) const final override {

			if (!file) {
				System::log()->fatal("Read access isn't permitted");
				return false;
			}

			if (offset == usz_MAX)
				f.fileSize += size;

			else if (offset > f.fileSize) {
				System::log()->fatal("File write out of bounds");
				return false;
			}

			else if(offset + size > f.fileSize)
				f.fileSize = offset + size;

			if(offset != usz_MAX)
				fseeko(file, offset, 0);

			return fwrite(v, 1, size, file);
		}
	};

	LocalFileSystem::LocalFileSystem(String localPath): 
		FileSystem(Array<FileAccess, 2>{ FileAccess::READ, FileAccess::READ_WRITE }), 
		localPath(localPath) {}

	const String &LocalFileSystem::getLocalPath() const {
		return localPath;
	}

	File *LocalFileSystem::open(FileInfo &info) {

		if (info.isFolder) {
			System::log()->fatal("Can't open a folder");
			return nullptr;
		}

		if (!info.isLocal()) 
			return openVirtual(info);

		return new CFile(info);
	}

	bool LocalFileSystem::make(FileInfo &file) {

		if (!file.isLocal()) {
			System::log()->fatal("Couldn't create a file in a virtual space");
			return false;
		}

		if (file.isFolder) {

			if (_mkdir(file.path.c_str())) {
				System::log()->fatal("Couldn't create local folder");
				return false;
			}

		} else {

			FILE *f{};
			if (fopen_s(&f, file.path.c_str(), "w")) {
				System::log()->fatal("Couldn't create local file");
				return false;
			}

			fclose(f);

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

		if (change == FileChange::ADD)
			make(file);

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