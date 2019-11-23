#include "system/file_system.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include <algorithm>

namespace oic {

	static constexpr c8 vroot[] = "~", lroot[] = ".";

	FileInfo::SizeType FileInfo::getFolders() const { return fileHint - folderHint; }
	FileInfo::SizeType FileInfo::getFiles() const { return fileEnd - fileHint; }
	FileInfo::SizeType FileInfo::getFileObjects() const { return fileEnd - folderHint; }
	bool FileInfo::isLocal() const { return path[0] == lroot[0]; }
	bool FileInfo::hasData() const { return fileSize != 0; }
	bool FileInfo::hasRegion(usz size, usz offset) const {
		return fileSize > 0 && fileSize > size + offset;
	}

    bool FileInfo::hasAccess(FileAccess flags) const {
		return hasAccess(access, flags);
    }

	bool FileInfo::hasAccess(FileAccess access, FileAccess flags) {
		return (u8(access) & u8(flags)) == u8(flags);
	}

	FileSystem::FileSystem(const Array<FileAccess, 2> &fileAccess): 
		fileAccess(fileAccess),
		virtualFileLut { { vroot, 0 } },
		virtualFiles {
			{
				FileInfo{
					vroot, vroot,
					0, 0, nullptr, 0, 0, 0, 0, 0, 0,
					fileAccess[false],
					true
				}
			}
		},
		localFileLut{ { lroot, 0 } },
		localFiles{
			FileInfo{
				lroot, lroot,
				0, 0, nullptr, 0, 0, 0, 0, 0, 0,
				fileAccess[true],
				true
			}
		}
	{ }

	//TODO: This can be "simplified" by a custom List class
	template<typename T>
	void addUnique(List<T> &arr, const T &t) {

		auto it = std::find(arr.begin(), arr.end(), t);

		if (it != arr.end())
			return;

		arr.push_back(t);
	}

	//TODO: This can be "simplified" by a custom List class
	template<typename T>
	void remove(List<T> &arr, const T &t) {

		auto it = std::find(arr.begin(), arr.end(), t);

		if (it == arr.end())
			return;

		arr.erase(it);
	}

    void FileSystem::addFileChangeCallback(FileChangeCallback callback) {
		oic::addUnique(callbacks, callback);
    }

    void FileSystem::removeFileChangeCallback(FileChangeCallback callback) {
		oic::remove(callbacks, callback);
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

	//TODO: What happens with "a/"? does it just turn into "a"?
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

    bool FileSystem::foreachFile(const FileInfo &info, FileCallback callback, bool recurse) {

        if(info.path == "" || !info.hasAccess(FileAccess::READ))
            return false;

        auto &arr = files(info.isLocal());

        for(usz i = info.folderHint, end = info.fileEnd; i != end; ++i)
            callback(this, arr[i]);

        if(recurse)
            for(usz i = info.folderHint; i != info.fileHint; ++i)
                foreachFile(arr[i], callback, true);

        return true;
    }

	const FileInfo &FileSystem::get(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			System::log()->fatal("File path should be in proper oic notation");

		if (apath[0] == '.') {

			auto ou = localFileLut.find(apath);

			if (ou == localFileLut.end())
				System::log()->fatal("Local file doesn't exist");

			return localFiles[ou->second];
		}

		auto ou = virtualFileLut.find(apath);

		if (ou == virtualFileLut.end())
			System::log()->fatal("Virtual file doesn't exist");

		return virtualFiles[ou->second];
	}

	FileInfo &FileSystem::get(const String &path) {
		return (FileInfo&)((const FileSystem*)this)->get(path);
	}

	bool FileSystem::exists(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		if (apath[0] == '~')
			return virtualFileLut.find(apath) != virtualFileLut.end();

		return localFileLut.find(apath) != localFileLut.end();
	}

	bool FileSystem::regionExists(const String &path, usz size, usz offset) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		if (apath[0] == '~') {

			auto it = virtualFileLut.find(apath);

			if (it == virtualFileLut.end())
				return false;

			return virtualFiles[it->second].hasRegion(size, offset);
		}

		auto it = localFileLut.find(apath);

		if (it == localFileLut.end())
			return false;

		return localFiles[it->second].hasRegion(size, offset);
	}

	void FileSystem::initLut() {

		FileInfo::SizeType i{};
		virtualFileLut.clear();

		for (const FileInfo &info : virtualFiles) {
			virtualFileLut[info.path] = i;
			++i;
		}

		i = 0;
		localFileLut.clear();

		for (const FileInfo &info : localFiles) {
			localFileLut[info.path] = i;
			++i;
		}

	}

	bool FileSystem::rem(const String &path) {
		FileInfo &inf = get(path);
		return remove(inf.id, inf.isLocal());
	}

	bool FileSystem::remove(FileHandle handle, bool isLocal) {

		if (handle == 0) {
			System::log()->fatal("Can't remove root node");
			return false;
		}

		FileInfo &inf = get(handle, isLocal);

		if (!inf.hasAccess(FileAccess::WRITE)) {
			System::log()->fatal("File access isn't allowed; write access is disabled");
			return false;
		}

		//Notify folders (clean up their children)

		FileInfo::SizeType fileObjects;

		if ((fileObjects = inf.getFileObjects()) != 0) {

			FileHandle folderEnd = inf.fileHint - 1;
			FileHandle folderStart = inf.folderHint;
			FileInfo::SizeType folders = inf.getFolders();

			FileHandle fileEnd = inf.fileEnd - 1 - folders;	//- folders since those will be removed
			FileHandle fileStart = inf.fileHint - folders;		//- folders since those will be removed

			for (FileHandle i = folderEnd; i >= folderStart; --i)
				remove(i, isLocal);

			for (FileHandle i = fileEnd; i >= fileStart; --i)
				remove(i, isLocal);
		}

		//Pass remove events

		FileInfo &fs = get(handle, isLocal);

		onFileChange(fs, FileChange::REM);

		for (FileChangeCallback cb : callbacks)
			cb(this, fs, FileChange::REM);

		//Remove from parent

		List<FileInfo> &arr = files(isLocal);
		auto &map = isLocal ? localFileLut : virtualFileLut;

		bool isFolder = fs.isFolder;
		FileInfo &parent = arr[fs.parent];

		if (isFolder)
			--parent.fileHint;

		--parent.fileEnd;

		//Remove from system

		map.erase(arr[handle].path);
		arr.erase(arr.begin() + handle);

		//TODO: Doesn't work :(
		//delete working/tools
		//delete working/out
		//delete working/oibaker
		//working/oibaker points to working/oibaker/src/main.cpp instead of working/oibaker/CMakeLists.txt

		for (FileHandle i = handle, j = FileHandle(arr.size()); i < j; ++i) {

			FileInfo &f = arr[i];
			FileHandle shouldDecrement = f.folderHint != 0;

			map[f.path] = --f.id;

			f.parent -= FileHandle(f.parent > handle);
			f.folderHint -= shouldDecrement;
			f.fileHint -= shouldDecrement;
			f.fileEnd -= shouldDecrement;
		}

		return true;
	}

	bool FileSystem::add(const String &path, bool isFolder) {

		String apath;

		if (!resolvePath(path, apath)) {
			System::log()->fatal("Invalid file path");
			return false;
		}

		if (exists(apath)) {
			System::log()->fatal("File already exists");
			return false;
		}

		bool isLocal = apath[0] == '.';

		List<String> parts;
		obtainPath(path, parts);

		//Go through subfiles and find the parent

		auto &arr = files(isLocal);
		FileInfo *parent = arr.data();

		for (usz i = 1, j = parts.size() - 1; j != usz_MAX && i < j; ++i) {

			String part = parts[i];
			bool found = false;

			for (usz k = parent->folderHint; k != parent->fileHint; ++k) {

				usz l = k - parent->id;
				if (parent[l].name == part) {
					parent = parent + l;
					found = true;
					break;
				}
			}

			//Mkdir

			if (!found) {
				String dpath = parent->path + "/" + part;

				if (!add(dpath, true)) {
					System::log()->fatal("Couldn't create subdirectory");
					return false;
				}

				parent = &get(dpath);
			}

		}

		if (!parent->hasAccess(FileAccess::WRITE)) {
			System::log()->fatal("Write into folder isn't supported");
			return false;
		}

		//Create file or directory

		String &part = *(parts.end() - 1);

		FileHandle handle = isFolder ? parent->fileHint : parent->fileEnd;

		//The file doesn't have folders/files yet
		if (handle == 0)
			System::log()->fatal("File hint is invalid");

		//Set up the parent's handles

		bool setParentHint = false;

		if (parent->getFolders() == 0 && isFolder) {			//Setup folder hint

			auto files = parent->getFiles();

			if (files == 0)
				parent->fileHint = handle + 1;
			else
				++parent->fileHint;

			parent->folderHint = handle;
			parent->fileEnd = handle + files + 1;

		} else if (parent->getFiles() == 0 && !isFolder) {		//Setup file hint

			FileHandle folders = parent->getFolders();

			if (folders == 0)
				parent->folderHint = handle;

			FileHandle ou = parent->folderHint;

			parent->fileHint = ou + folders;
			parent->fileEnd = ou + folders + 1;

		} else {												//Increase hints
			parent->fileHint += FileHandle(isFolder);
			++parent->fileEnd;
		}

		//Update all file handles

		auto &lut = isLocal ? localFileLut : virtualFileLut;

		FileHandle pid = parent->id;	//All ids before handle stay the same, so parent->id and parent->parent is always valid

		for (FileHandle i = 0, j = size(isLocal); i < j; ++i) {

			FileInfo &f = arr[i];

			FileHandle shouldIncrement(
				f.id != pid && (f.fileEnd > handle || (f.fileEnd == handle && f.parent >= pid))
			);

			f.folderHint += shouldIncrement;
			f.fileHint += shouldIncrement;
			f.fileEnd += shouldIncrement;

			if (f.id >= handle)
				lut[f.path] = ++f.id;

			f.parent += FileHandle(f.parent >= handle);
		}

		//Obtain where the folder's children should be located

		FileHandle hint = isFolder ? parent->fileEnd : 0;

		if (isFolder && parent->getFolders() > 1 && !setParentHint)
			hint = arr[parent->fileHint - 2].fileEnd;

		//The folder doesn't have a place to go

		if (hint == 0 && isFolder)
			System::log()->fatal("Folder doesn't have place it could be allocated in");

		//Add to system

		FileInfo info {
			apath, part,
			0, 0, nullptr, 0,
			parent->id, handle, hint, hint, hint,
			parent->access, isFolder
		};

		lut[apath] = handle;
		arr.insert(arr.begin() + handle, info);

		//Send update

		FileInfo &fi = arr[handle];
		onFileChange(fi, FileChange::ADD);

		for (FileChangeCallback cb : callbacks)
			cb(this, fi, FileChange::ADD);

		return true;
	}

	bool FileSystem::upd(const String &path) {

		FileInfo &file = get(path);
		onFileChange(file, FileChange::UPD);

		for (FileChangeCallback cb : callbacks)
			cb(this, file, FileChange::UPD);

		return true;
	}

	bool FileSystem::mov(const String &path, const String &npath) {

		if (path.substr(0, path.find_last_of('/')) != npath.substr(0, npath.find_last_of('/'))) {
			System::log()->fatal("Cannot move a file to a different folder: Not supported yet");
			return false;
		}

		FileInfo &info = get(path);
		rename(info, npath, true);

		if (info.isFolder)
			foreachFile(info, [] (FileSystem *f, FileInfo &fi) -> void {

				f->rename(fi, f->get(fi.parent, fi.isLocal()).path + "/" + fi.name, false);

			}, true);

		onFileChange(info, FileChange::MOV);

		for (FileChangeCallback cb : callbacks)
			cb(this, info, FileChange::MOV);

		return true;
	}

	FileInfo::SizeType FileSystem::size(bool isLocal) const {
		return FileInfo::SizeType(localFiles.size() * isLocal + virtualFiles.size() * (!isLocal));
	}

	const List<FileInfo> &FileSystem::getFiles(bool isLocal) const {
		return isLocal ? localFiles : virtualFiles;
	}

	List<FileInfo> &FileSystem::files(bool isLocal) {
		return (List<FileInfo>&) getFiles(isLocal);
	}

	void FileSystem::begin() { lock.lock(); }
	void FileSystem::end() { lock.unlock(); }

	void FileSystem::rename(FileInfo &info, const String &path, bool setName) {

		auto &map = info.isLocal() ? localFileLut : virtualFileLut;

		map.erase(info.path);

		info.path = path;
		map[info.path] = info.id;

		if (setName)
			info.name = info.path.substr(info.path.find_last_of('/') + 1);

	}

	const FileInfo &FileSystem::get(FileHandle id, bool isLocal) const {

		if (id >= size(isLocal))
			System::log()->fatal("FileHandle doesn't exist");

		return getFiles(isLocal)[id];
	}

	FileInfo &FileSystem::get(FileHandle id, bool isLocal) {
		return (FileInfo&)((const FileSystem*)this)->get(id, isLocal);
	}

}