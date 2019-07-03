#include "system/file_system.hpp"
#include "system/system.hpp"
#include "error/ocore.hpp"
#include "system/log.hpp"
#include <algorithm>

namespace oic {

    usz FileInfo::getFolders() const { return fileHint - folderHint; }
    usz FileInfo::getFiles() const { return fileEnd - fileHint; }
	usz FileInfo::getFileObjects() const { return fileEnd - folderHint; }
	bool FileInfo::isVirtual() const { return path[0] == '~'; }
	bool FileInfo::hasData() const { return fileSize != 0; }

    bool FileInfo::hasAccess(FileAccess flags) const {
        return (u8(access) & u8(flags)) == u8(flags);
    }

    FileSystem::FileSystem(bool allowLocalFiles): allowLocalFiles(allowLocalFiles) {
        initFiles();
		resetLut();
    }

	//TODO: This can be "simplified" by a custom List class
	template<typename T>
	void addUnique(List<T> &arr, const T &t) {

		auto it = std::find(arr.begin(), arr.end(), t);

		if (it != arr.end())
			return;

		arr.push_back(it, t);
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

	void FileSystem::notifyFileChange(const String &path, bool isRemoved) {

		String apath;

		if (!resolvePath(path, apath)) {
			System::log()->fatal(errors::fs::invalid);
			return;
		}

		//const auto &arr = info.isVirtual() ? virtualFiles : localFiles;

		//Remove the file and children
		//Update the handles of everyone

		if (isRemoved) {

			//TODO: Removing file the array doesn't work, because it needs to update the fileHint

			/*if (!exists(apath)) {
				System::log()->fatal(errors::fs::nonExistent);
				return;
			}

			
			for (FileRemoveCallback cb : removeCallbacks)
				cb(this, get(apath));

			onFileRemove(get(apath));

			localFileLut.erase(apath);*/

			System::log()->fatal(errors::fs::notSupported);
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

		auto it = apath[0] == '.' ? localFileLut.find(apath) : virtualFileLut.find(apath);
		FileInfo &file = apath[0] == '.' ? localFiles[it->second] : virtualFiles[it->second];
		onFileChange(file, false);

		for (FileChangeCallback cb : callbacks)
			cb(this, file, false);

	}

}