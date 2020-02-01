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

	class CFile : public File {

	private:

		FILE *file{};

		virtual ~CFile() { 
			fclose(file); 
			file = nullptr;
		}

	public:

		CFile(FileSystem *fs, const FileInfo &f, ns timeout, ns retry): File(fs, f) {
		
			const char *accessFlags = "rb";

			if (f.hasAccess(FileAccess::WRITE))
				accessFlags = "a+b";

			while (timeout >= retry) {

				if (fopen_s(&file, f.path.c_str(), accessFlags) != 0 || !file) {
					oic::System::wait(retry);
					timeout -= retry;
				}

				else {
					isOpen = true;
					timeout = 0;
				}
			}

			if(!isOpen)
				System::log()->fatal("File can't be opened");
		}

		bool read(void *v, FileSize size, FileSize offset) const final override {

			if (offset + size > f.fileSize) {
				System::log()->fatal("File read is out of bounds");
				return false;
			}

			fseeko(file, offset, 0);
			return fread(v, 1, size, file);
		}

		bool write(const void *v, FileSize size, FileSize offset) final override {

			if (offset == usz_MAX)
				;

			else if (offset > f.fileSize) {
				System::log()->fatal("File write out of bounds");
				return false;
			}

			hasWritten = true;

			if(offset != usz_MAX)
				fseeko(file, offset, 0);

			return fwrite(v, 1, size, file);
		}
	};

	LocalFileSystem::LocalFileSystem(String localPath): 
		FileSystem(FileAccess::READ), localPath(localPath) {}

	const String &LocalFileSystem::getLocalPath() const {
		return localPath;
	}

	File *LocalFileSystem::open(const FileInfo &info, ns timeout, ns retry) {

		if (info.isFolder()) {
			System::log()->fatal("Can't open a folder");
			return nullptr;
		}

		if (!info.isLocal()) 
			return openVirtual(info);

		return new CFile(this, info, timeout, retry);
	}

	bool LocalFileSystem::makeLocal(const String &file, bool isFolder) {

		if (isFolder) {

			if (_mkdir(file.c_str()) != 0) {
				System::log()->fatal("Couldn't create local folder");
				return false;
			}

		} else {

			FILE *f{};
			if (fopen_s(&f, file.c_str(), "w") != 0 || !f) {
				System::log()->fatal("Couldn't create local file");
				return false;
			}

			fclose(f);
		}

		return true;
	}

	bool LocalFileSystem::delLocal(const String &path) {

		const FileInfo inf = get(path);

		if (inf.isFolder()) {

			if (_rmdir(path.c_str()) != 0) {
				System::log()->fatal("Couldn't delete local folder");
				return false;
			}

		} else {

			if (::remove(path.c_str()) != 0) {
				System::log()->fatal("Couldn't delete local file");
				return false;
			}

		}

		return true;
	}

	void LocalFileSystem::onFileChange(const FileInfo &file, FileChange change) {

		if (change == FileChange::DEL)
			return;

		if (!file.isLocal()) {
			onVirtualFileChange(file, change);
			return;
		}
	}

	const FileInfo LocalFileSystem::local(const String &path) const {

		String apath;

		if (!resolvePath(path, apath)) {
			oic::System::log()->fatal("Invalid path passed to LocalFileSystem");
			return {};
		}

		struct stat v;

		if (stat(path.c_str(), &v)) {
			oic::System::log()->fatal("Local file not found");
			return {};
		}

		const bool isFile = S_ISREG(v.st_mode);
		u8 flags{};

		if (!isFile)
			flags |= u8(FileFlags::IS_FOLDER);

		if (v.st_mode & _S_IREAD)
			flags |= u8(FileFlags::READ);

		if (v.st_mode & _S_IWRITE)
			flags |= u8(FileFlags::WRITE);

		return FileInfo {
			path, path.substr(path.find_last_of('/') + 1),
			v.st_mtime, nullptr,
			isFile ? FileSize(v.st_size) : 0,
			0, 0, 0, 0, FileFlags(flags)
		};
	}

	bool LocalFileSystem::hasLocal(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		struct stat v;

		if (stat(path.c_str(), &v))
			return false;

		return true;
	}

	bool LocalFileSystem::hasLocalRegion(const String &path, FileSize size, FileSize offset) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		struct stat v;

		if (stat(path.c_str(), &v))
			return false;

		return usz(offset) + size > usz(v.st_size) * bool(S_ISREG(v.st_mode));
	}
}