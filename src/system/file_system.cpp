#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "system/file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include <algorithm>
#include <cstring>

namespace oic {

	static constexpr c8 vroot[] = "~", lroot[] = ".";

	FileInfo::SizeType FileInfo::getFolders() const { return fileHint - folderHint; }
	FileInfo::SizeType FileInfo::getFiles() const { return fileEnd - fileHint; }
	FileInfo::SizeType FileInfo::getFileObjects() const { return fileEnd - folderHint; }
	bool FileInfo::isFolder() const { return hasFlags(FileFlags::IS_FOLDER); }
	bool FileInfo::isVirtual() const { return hasFlags(FileFlags::IS_VIRTUAL); }
	bool FileInfo::isLocal() const { return !isVirtual(); }
	bool FileInfo::hasData() const { return fileSize != 0; }

	bool FileInfo::hasRegion(FileSize size, FileSize offset) const {
		return fileSize > size + offset;
	}

    bool FileInfo::hasAccess(FileAccess f) const {
		return hasFlags(flags, FileFlags(f));
    }

    bool FileInfo::hasFlags(FileFlags f) const {
		return hasFlags(flags, f);
    }

	bool FileInfo::hasFlags(FileFlags access, FileFlags flags) {
		return (u8(access) & u8(flags)) == u8(flags);
	}

	File::~File() {
		if (hasWritten)
			fs->update(f.path);
	}

	FileSystem::FileSystem(const FileAccess virtualFileAccess): 
		virtualFiles {
			{
				FileInfo{
					vroot, vroot,
					0, nullptr, 0, 0, 0, 0, 0,
					FileFlags(u8(virtualFileAccess) | u8(FileFlags::IS_FOLDER) | u8(FileFlags::IS_VIRTUAL))
				}
			}
		},
		virtualFileLut { { vroot, 0 } }
	{ }

    void FileSystem::addFileChangeCallback(FileChangeCallback callback, const String &path, void *ptr) {

		String apath;

		if (!resolvePath(path, apath) || callbacks.find(apath) != callbacks.end())
			return;

		callbacks[apath] = { callback, ptr };
		startFileWatcher(apath);
    }

    void FileSystem::removeFileChangeCallback(const String &path) {

		String apath;

		if (!resolvePath(path, apath))
			return;

		auto it = callbacks.find(apath);

		if (it == callbacks.end())
			return;

		endFileWatcher(apath);
		callbacks.erase(it);
    }

	usz FileSystem::obtainPath(const String &path, List<String> &splits) {

		auto beg = path.begin();
		auto end = path.end();

		splits.reserve(std::count(beg, end, '/') + 1);
		auto prev = beg;
		usz i{}, total{};

		for (auto it = beg, last = end - 1; it != end; ++it)
			if (*it == '/') {

				if ((it - prev) == 2 && *prev == '.' && *(prev + 1) == '.') {        //Erase previous if ..

					if (splits.size() <= 1)
						return false;

					total -= splits[splits.size() - 1].size();
					splits.erase(splits.end() - 1);

				} else if ((it - prev) != 1 || *prev != '.' || prev == beg) {         //Add next if not .
					splits.push_back(String(prev, it));
					total += it - prev;
				}

				prev = it;
				++prev;
				++i;

			} else if (it == last) {                                                 //Add last parameter
				splits.push_back(String(prev, end));
				total += end - prev;
			}

		return total;
	}

	//TODO: "a/" should turn into "a"
    bool FileSystem::resolvePath(const String &path, String &outPath) const {

		//Force correct paths

        if(path.size() == 0 || path.find('\\') != String::npos)
            return false;

		//Skip path parsing if there's no ./ and ../

		if (path.size() == 1 || path.find("/../") == String::npos || path.find("/./") == String::npos) {
			outPath = path;
			return (outPath[0] == '~' || outPath[0] == '.') && (outPath.size() == 1 || outPath[1] == '/');
		}

        //Split into sub paths
        
        List<String> splits;
		usz total = obtainPath(path, splits);

        //Combine into final string

        outPath = String(total + splits.size() - 1, '/');
        total = 0;

        for(String &str : splits){
            std::memcpy(outPath.data() + total, str.data(), str.size());
            total += str.size() + 1;
        }

        //Validate

        return (outPath[0] == '~' || outPath[0] == '.') && (outPath.size() == 1 || outPath[1] == '/');
    }

    bool FileSystem::foreachFile(const String &path, FileCallback callback, bool recurse, void *data) {

        if(path == "")
            return false;

		const FileInfo info = get(path);

		if (info.isLocal()) {

			for(const String &file : localFileObjects(path))
				callback(this, get(file), data);

			if(recurse)
				for(const String &folder : localDirectories(path))
					foreachFile(folder, callback, true, data);

			return true;
		}

        auto &arr = virtualFiles;

        for(FileHandle i = info.folderHint, end = info.fileEnd; i != end; ++i)
            callback(this, arr[i], data);

        if(recurse)
            for(FileHandle i = info.folderHint; i != info.fileHint; ++i)
                foreachFile(arr[i].path, callback, true, data);

        return true;
    }

	const FileInfo FileSystem::get(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			System::log()->fatal("File path should be in proper oic notation");

		if (apath[0] == '.')
			return local(apath);

		auto ou = virtualFileLut.find(apath);

		if (ou == virtualFileLut.end())
			System::log()->fatal("Virtual file doesn't exist");

		return virtualFiles[ou->second];
	}

	bool FileSystem::exists(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		if (apath[0] == '~')
			return virtualFileLut.find(apath) != virtualFileLut.end();

		return hasLocal(apath);
	}

	bool FileSystem::regionExists(const String &path, FileSize size, FileSize offset) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		if (apath[0] == '~') {

			auto it = virtualFileLut.find(apath);

			if (it == virtualFileLut.end())
				return false;

			return virtualFiles[it->second].hasRegion(size, offset);
		}

		return hasLocalRegion(apath, size, offset);
	}

	void FileSystem::initLut() {

		FileInfo::SizeType i{};
		virtualFileLut.clear();

		for (const FileInfo &info : virtualFiles) {
			virtualFileLut[info.path] = i;
			++i;
		}
	}

	bool FileSystem::remove(const String &path, bool isCallback) {

		String apath;

		if (!resolvePath(path, apath)) {
			System::log()->fatal("Invalid path");
			return false;
		}

		FileFlags flags = apath[0] == '~' ? FileFlags::IS_VIRTUAL: FileFlags::NONE;

		if (!isCallback) {

			const FileInfo inf = get(apath);

			if (!inf.hasAccess(FileAccess::WRITE) || inf.path.size() == 1) {
				System::log()->fatal("File access isn't allowed; write access is disabled");
				return false;
			}

			//Remove children

			if (inf.isVirtual()) {

				FileInfo::SizeType fileObjects;

				if ((fileObjects = inf.getFileObjects()) != 0) {

					FileHandle folderEnd = inf.fileHint - 1;
					FileHandle folderStart = inf.folderHint;
					FileInfo::SizeType folders = inf.getFolders();

					FileHandle fileEnd = inf.fileEnd - 1 - folders;		//- folders since those will be removed
					FileHandle fileStart = inf.fileHint - folders;		//- folders since those will be removed

					for (FileHandle i = folderEnd; i >= folderStart; --i)
						remove(virtualFiles[i].path);

					for (FileHandle i = fileEnd; i >= fileStart; --i)
						remove(virtualFiles[i].path);
				}
			}
		}

		//Pass remove events

		FileInfo inf {
			apath, apath.substr(apath.find_last_of('/') + 1),
			0, nullptr, 0, 0, 0, 0, 0,
			flags
		};

		onFileChange(inf, FileChange::DEL);

		for (auto &cb : callbacks)
			cb.second.first(this, inf, FileChange::DEL, cb.second.second);

		//Remove actual file

		if (!isCallback) {

			if (inf.isVirtual()) {

				//Remove from parent

				List<FileInfo> &arr = virtualFiles;
				auto &map = virtualFileLut;

				FileInfo &parent = arr[inf.parent];

				if (inf.isFolder())
					--parent.fileHint;

				--parent.fileEnd;

				//Remove from system

				FileHandle handle = map.find(inf.path)->second;
				map.erase(inf.path);
				arr.erase(arr.begin() + handle);

				for (FileHandle i = handle, j = FileHandle(arr.size()); i < j; ++i) {

					FileInfo &f = arr[i];
					FileHandle shouldDecrement = f.folderHint != 0;

					f.parent -= FileHandle(f.parent > handle);
					f.folderHint -= shouldDecrement;
					f.fileHint -= shouldDecrement;
					f.fileEnd -= shouldDecrement;
				}
			} else delLocal(inf.path);
		}

		return true;
	}

	bool FileSystem::add(const String &path, bool isFolder, bool isCallback) {

		String apath;

		if (!resolvePath(path, apath)) {
			System::log()->fatal("Invalid file path");
			return false;
		}

		if (exists(apath))
			return true;

		bool isLocal = apath[0] == '.';

		List<String> parts;
		obtainPath(path, parts);

		//Go through subfiles and find the parent

		FileHandle pid{};
		FileInfo parent;

		if (!isCallback) {

			if (!isLocal) {

				auto &arr = virtualFiles;
				parent = arr[0];

				String dpath = "~";

				for (usz i = 1, j = parts.size() - 1; i < j; ++i) {

					const String part = parts[i];
					bool found = false;

					dpath += "/" + part;

					for (FileHandle k = parent.folderHint; k != parent.fileHint; ++k) {

						if (arr[k].name == part) {
							parent = arr[pid = k];
							found = true;
							break;
						}
					}

					//Mkdir

					if (!found) {

						if (!add(dpath, true)) {
							System::log()->fatal("Couldn't create subdirectory");
							return false;
						}

						parent = get(dpath);
					}
				}

				if (parent.path.empty())
					parent = get(dpath);

			} else {

				String dpath = ".";

				for (usz i = 1, j = parts.size() - 1; i < j; ++i) {

					const String part = parts[i];
					dpath += "/" + part;

					//Mkdir

					if (!exists(dpath)) {

						if (!add(dpath, true)) {
							System::log()->fatal("Couldn't create subdirectory");
							return false;
						}

						parent = get(dpath);
					}
				}

				if (parent.path.empty())
					parent = get(dpath);
			}

			if (!parent.hasAccess(FileAccess::WRITE)) {
				System::log()->fatal("Write into folder isn't supported");
				return false;
			}

			//Create file or directory

			String &part = *(parts.end() - 1);

			if (!isLocal) {

				FileHandle handle = isFolder ? parent.fileHint : parent.fileEnd;

				//The file doesn't have folders/files yet
				if (handle == 0) {
					System::log()->fatal("File hint is invalid");
					return false;
				}

				//Set up the parent's handles

				auto &p = virtualFiles[pid];

				bool setParentHint = false;

				if (parent.getFolders() == 0 && isFolder) {			//Setup folder hint

					const FileHandle files = parent.getFiles();

					if (files == 0)
						p.fileHint = handle + 1;
					else
						++p.fileHint;

					p.folderHint = handle;
					p.fileEnd = handle + files + 1;

				} else if (parent.getFiles() == 0 && !isFolder) {		//Setup file hint

					const FileHandle folders = parent.getFolders();
					const FileHandle ou = parent.folderHint;

					if (folders == 0)
						p.folderHint = handle;

					p.fileHint = ou + folders;
					p.fileEnd = ou + folders + 1;

				} else {												//Increase hints
					p.fileHint += FileHandle(isFolder);
					++p.fileEnd;
				}

				//Update all file handles

				auto &lut = virtualFileLut;
				auto &arr = virtualFiles;

				for (FileHandle i = 0, j = virtualSize(); i < j; ++i) {

					FileInfo &f = arr[i];

					FileHandle shouldIncrement(
						i != pid && (f.fileEnd > handle || (f.fileEnd == handle && f.parent >= pid))
					);

					f.folderHint += shouldIncrement;
					f.fileHint += shouldIncrement;
					f.fileEnd += shouldIncrement;

					if (i >= handle)
						lut[f.path] = i + 1;

					f.parent += FileHandle(f.parent >= handle);
				}

				//Obtain where the folder's children should be located

				FileHandle hint = isFolder ? parent.fileEnd : 0;

				if (isFolder && parent.getFolders() > 1 && !setParentHint)
					hint = arr[parent.fileHint - 2].fileEnd;

				//The folder doesn't have a place to go

				if (hint == 0 && isFolder) {
					System::log()->fatal("Folder doesn't have place it could be allocated in");
					return false;
				}

				//Add to system

				FileInfo info {
					apath, part,
					0, nullptr, 0,
					pid, hint, hint, hint,
					FileFlags(
						isFolder ? u8(parent.flags) : (u8(parent.flags) & ~u8(FileFlags::IS_FOLDER))
					)
				};

				lut[apath] = handle;
				arr.insert(arr.begin() + handle, info);

			}
			else makeLocal(apath, isFolder);
		}

		//Send update

		const FileInfo fi = get(apath);
		onFileChange(fi, FileChange::ADD);

		for (auto &cb : callbacks)
			cb.second.first(this, fi, FileChange::ADD, cb.second.second);

		return true;
	}

	bool FileSystem::update(const String &path) {

		const FileInfo &file = get(path);
		onFileChange(file, FileChange::UPDATE);

		for (auto &cb : callbacks)
			cb.second.first(this, file, FileChange::UPDATE, cb.second.second);

		return true;
	}

	bool FileSystem::mov(const String &path, const String &npath, bool isCallback) {

		if (path.substr(0, path.find_last_of('/')) != npath.substr(0, npath.find_last_of('/'))) {
			System::log()->fatal("Cannot move a file to a different folder: Not supported yet");
			return false;
		}

		if (!isCallback) {

			const FileInfo &info = get(path);
			rename(info, npath, true);

			if (info.isFolder())
				foreachFile(path, [](FileSystem *f, const FileInfo &fii, void *np) -> void {

					const FileInfo fi = fii;
					f->rename(fi, (const char*)np + String("/") + fi.name, false);

				}, true, (void*)npath.c_str());

		}

		const FileInfo &info = get(npath);
		onFileChange(info, FileChange::MOVE);

		for (auto &cb : callbacks)
			cb.second.first(this, info, FileChange::MOVE, cb.second.second);

		return true;
	}

	void FileSystem::lock() { mutex.lock(); }
	void FileSystem::unlock() { mutex.unlock(); }

	void FileSystem::rename(const FileInfo &info, const String &path, bool setName) {

		auto &map = virtualFileLut;
		auto &arr = virtualFiles;

		FileHandle id = map[info.path];
		map.erase(info.path);

		arr[id].path = path;
		map[path] = id;

		if (setName)
			arr[id].name = info.path.substr(info.path.find_last_of('/') + 1);
	}

	bool FileSystem::read(const String &file, u8 *address, FileSize size, FileSize offset) {

		if (File *f = open(file)) {
			bool success = f->read(address, size, offset);
			close(f);
			return success;
		}

		return false;
	}

	bool FileSystem::read(const String &path, Buffer &buffer, FileSize size, FileSize offset) {

		if (!size) {
			const FileInfo file = get(path);
			size = file.fileSize - (offset >= file.fileSize ? 0 : offset);
		}

		buffer.resize(size);
		return read(path, buffer.data(), size, offset);
	}

	void FileSystem::close(File *f) { 
		delete f;
	}

	bool FileSystem::write(const String &path, const u8 *address, FileSize size, FileSize offset) {

		if (File *f = open(path)) {
			bool success = f->write(address, size, offset);
			close(f);
			return success;
		}

		return false;
	}
	
	bool FileSystem::write(const String &path, const Buffer &buffer, FileSize size, usz bufferOffset, FileSize fileOffset) {

		if (bufferOffset + size > buffer.size())
			oic::System::log()->fatal("Buffer out of bounds");

		if (!size)
			size = buffer.size() - bufferOffset;

		return write(path, buffer.data() + bufferOffset, size, fileOffset);
	}
}