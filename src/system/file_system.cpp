#include "system/file_system.hpp"
#include "system/system.hpp"
#include "error/ocore.hpp"
#include "system/log.hpp"
#include <algorithm>

namespace oic {

    usz FileInfo::getFolders() const { return fileHint - folderHint; }
    usz FileInfo::getFiles() const { return fileEnd - fileHint; }
	usz FileInfo::getFileObjects() const { return fileEnd - folderHint; }

    bool FileInfo::hasAccess(FileAccess flags) const {
        return (u8(access) & u8(flags)) == u8(flags);
    }

    FileSystem::FileSystem() {
        //initFiles();
		resetLut();
    }

    void FileSystem::addFileChangeCallback(FileCallback callback) {

        auto it = std::find(callbacks.begin(), callbacks.end(), callback);

        if(it != callbacks.end())
            return;

        callbacks.insert(it, callback);

    }

    void FileSystem::removeFileChangeCallback(FileCallback callback) {

        auto it = std::find(callbacks.begin(), callbacks.end(), callback);

        if(it != callbacks.end())
            return;

        callbacks.erase(it);

    }

	//TODO: What happens with "a/"? does it just turn into "a"?
    bool FileSystem::resolvePath(const String &path, String &outPath) const {

        if(path.size() == 0 || path.find('\\') != String::npos)
            return false;

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

        const auto &arr = info.isVirtual ? virtualFiles : localFiles;

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

		if (apath[0] == '.' && supportsLocalFiles()) {

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

	bool FileSystem::exists(const String &path) const {

		String apath;

		if (!resolvePath(path, apath))
			return false;

		if (apath[0] == '~')
			return virtualFileLut.find(apath) != virtualFileLut.end();
		else if (supportsLocalFiles())
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

		if (!supportsLocalFiles())
			return;

		localFileLut.clear();

		i = 0;

		for (const FileInfo &info : localFiles) {
			localFileLut[info.path] = i;
			++i;
		}

	}

}