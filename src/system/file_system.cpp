#include "system/file_system.hpp"
#include "system/system.hpp"
#include "error/ocore.hpp"
#include "system/log.hpp"
#include <algorithm>

namespace oic {

	FileHandle FileInfo::getFolders() const { return fileHint - folderHint; }
	FileHandle FileInfo::getFiles() const { return fileEnd - fileHint; }
	FileHandle FileInfo::getFileObjects() const { return fileEnd - folderHint; }
	bool FileInfo::isVirtual() const { return path[0] == '~'; }
	bool FileInfo::hasData() const { return fileSize != 0; }

    bool FileInfo::hasAccess(FileAccess flags) const {
        return (u8(access) & u8(flags)) == u8(flags);
    }

	FileSystem::FileSystem(bool allowLocalFiles): allowLocalFiles(allowLocalFiles), virtualFileLut { { "~", 0 } } {
	
		if (allowLocalFiles) {
			localFileLut = { { ".", 0 } };
			localFiles = {
				FileInfo{
					".",
					0, 0, nullptr, 0, 0, 0, 0, 0, 0,
					FileAccess::READ_WRITE,
					true
				}
			};
		}

		virtualFiles = {
			FileInfo{
				"~",
				0, 0, nullptr, 0, 0, 0, 0, 0, 0,
				FileAccess::READ_WRITE,
				true
			}
		};

	}

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
		addUnique(callbacks, callback);
    }

    void FileSystem::removeFileChangeCallback(FileChangeCallback callback) {
		oic::remove(callbacks, callback);
    }

	//TODO: What happens with "a/"? does it just turn into "a"?
    bool FileSystem::resolvePath(const String &path, String &outPath) const {

		//Force correct paths

        if(path.size() == 0 || path.find('\\') != String::npos)
            return false;

		//Skip path parsing if there's no .

		if (path.size() == 1 || path.find('.', 1) == String::npos) {
			outPath = path;
			return (outPath[0] == '~' || outPath[0] == '.') && (outPath.size() == 1 || outPath[1] == '/');
		}

		//TODO: Check if it contains ../ or ./

        //Split into sub paths

        auto beg = path.begin();
        auto end = path.end();
        
        List<String> splits;
        splits.reserve(std::count(beg, end, '/') + 1);
        auto prev = beg;
        usz i{}, total{};

        for(auto it = beg, last = end - 1; it != end; ++it)
            if(*it == '/') {
                
                if((it - prev) == 2 && *prev == '.' && *(prev + 1) == '.') {        //Erase previous if ..

                    if(splits.size() <= 1)
                        return false;

                    total -= splits[splits.size() - 1].size();
                    splits.erase(splits.end() - 1);

                } else if((it - prev) != 1 || *prev != '.' || prev == beg){         //Add next if not .
                    splits.push_back(String(prev, it));
                    total += it - prev;
                }
                    
                prev = it;
                ++prev;
                ++i;
            } else if(it == last) {                                                 //Add last parameter
                splits.push_back(String(prev, end));
                total += end - prev;
            }

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

        const auto &arr = info.isVirtual() ? virtualFiles : localFiles;

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
			System::log()->fatal(errors::fs::invalid);

		if (apath[0] == '.' && allowLocalFiles) {

			auto ou = localFileLut.find(apath);

			if (ou == localFileLut.end())
				System::log()->fatal(errors::fs::nonExistent);

			return localFiles[ou->second];
		}
		else if(apath[0] != '.')
			System::log()->fatal(errors::fs::notSupported);

		auto ou = virtualFileLut.find(apath);

		if (ou == virtualFileLut.end())
			System::log()->fatal(errors::fs::nonExistent);

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
		else if (allowLocalFiles)
			return localFileLut.find(apath) != localFileLut.end();

		return false;
	}

	void FileSystem::resetLut() {

		virtualFileLut.clear();

		FileInfo::SizeType i{};

		for (const FileInfo &info : virtualFiles) {
			virtualFileLut[info.path] = i;
			++i;
		}

		if (!allowLocalFiles)
			return;

		localFileLut.clear();

		i = 0;

		for (const FileInfo &info : localFiles) {
			localFileLut[info.path] = i;
			++i;
		}

	}

	void FileSystem::notifyFileRemove(FileHandle handle, bool isLocal) {

		if (handle == 0)
			System::log()->fatal(errors::fs::illegal);

		FileInfo *inf = &get(handle, isLocal);
		
		//Notify folders (clean up their children)

		if (inf->fileHint != 0) {

			FileHandle folderEnd = inf->fileHint - 1;
			FileHandle folderStart = inf->folderHint;
			FileInfo::SizeType folders = inf->getFolders();

			FileHandle fileEnd = inf->fileEnd - 1 - folders;	//- folders since those will be removed
			FileHandle fileStart = inf->fileHint - folders;		//- folders since those will be removed

			for (FileHandle i = folderEnd; i >= folderStart; --i)
				notifyFileRemove(i, isLocal);

			for (FileHandle i = fileEnd; i >= fileStart; --i)
				notifyFileRemove(i, isLocal);
		}

		//Pass remove events

		FileInfo &fs = get(handle, isLocal);

		for (FileChangeCallback cb : callbacks)
			cb(this, fs, true);

		onFileChange(fs, true);

		//Remove from parent

		bool isFolder = fs.isFolder;
		FileInfo &parent = isLocal ? localFiles[fs.parent] : virtualFiles[fs.parent];

		if (isFolder)
			--parent.fileHint;

		--parent.fileEnd;

		//Remove from system

		if (!isLocal) {
			virtualFileLut.erase(virtualFiles[handle].path);
			virtualFiles.erase(virtualFiles.begin() + handle);
		} else if (allowLocalFiles) {
			localFileLut.erase(localFiles[handle].path);
			localFiles.erase(localFiles.begin() + handle);
		} else
			System::log()->fatal(errors::fs::notSupported);

	}

	void FileSystem::notifyFileChange(const String &path, bool isRemoved) {

		String apath;

		if (!resolvePath(path, apath)) {
			System::log()->fatal(errors::fs::invalid);
			return;
		}

		bool isLocal = apath[0] == '.';
		auto &arr = isLocal ? localFiles : virtualFiles;
		auto &map = isLocal ? localFileLut : virtualFileLut;

		//Remove the file and children
		//Update the handles of everyone

		if (isRemoved) {

			FileHandle loc = get(path).id;
			notifyFileRemove(loc, isLocal);

			for (FileHandle i = loc, end = isLocal ? localSize() : virtualSize(); i != end; ++i) {

				FileInfo &f = arr[i];
				FileInfo &parent = get(f.path.substr(0, f.path.find_last_of('/')));

				map[f.path] = i;

				f.id = i;
				f.parent = parent.id;
				f.folderHint = f.fileHint = f.fileEnd = 0;

				if (parent.folderHint == 0 && f.isFolder) {
					parent.folderHint = i;
					parent.fileHint = i;
					parent.fileEnd = i;
				} else if (parent.fileHint == 0 && !f.isFolder) {

					if(parent.folderHint == 0)
						parent.folderHint = i;

					parent.fileHint = i;
					parent.fileEnd = i;
				}

				if (f.isFolder)
					++parent.fileHint;

				++parent.fileEnd;
			}

			return;
		}

		//Create file

		if (!exists(apath)) {

			//TODO: Appending to the array doesn't work, because it needs to update the fileHint

			/*usz ou = usz(localFiles.end() - localFiles.begin());

			localFiles.push_back(FileInfo{});
			FileInfo &file = localFiles[ou];

			onFileChange(file);
			localFileLut[apath] = FileInfo::SizeType(ou);

			for (FileCallback cb : callbacks)
				cb(this, file);*/

			System::log()->fatal(errors::fs::notSupported);
			return;
		}

		//Update file

		auto it = map.find(apath);
		FileInfo &file = arr[it->second];
		onFileChange(file, false);

		for (FileChangeCallback cb : callbacks)
			cb(this, file, false);

	}

	FileInfo::SizeType FileSystem::localSize() const {
		return FileInfo::SizeType(localFiles.size());
	}

	FileInfo::SizeType FileSystem::virtualSize() const {
		return FileInfo::SizeType(virtualFiles.size());
	}
	
	void FileSystem::addLocal(FileInfo &file) {

		if (!allowLocalFiles)
			return;

		localFiles.push_back(file);
		localFileLut[file.path] = file.id;
	}

	const FileInfo &FileSystem::get(FileHandle id, bool isLocal) const {

		if (isLocal && !allowLocalFiles)
			System::log()->fatal(errors::fs::notSupported);

		if ((isLocal && id >= localSize()) || (!isLocal && id >= virtualSize()))
			System::log()->fatal(errors::fs::nonExistent);

		return isLocal ? localFiles[id] : virtualFiles[id];
	}

	FileInfo &FileSystem::get(FileHandle id, bool isLocal) {
		return (FileInfo&)((const FileSystem*)this)->get(id, isLocal);
	}

}