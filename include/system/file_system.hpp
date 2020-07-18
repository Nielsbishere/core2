#pragma once
#include <mutex>
#include "types/types.hpp"

namespace oic {

    //!The access flags of a file
	//NONE: The file doesn't have data or children to be read or written
    //READ: The file's data can be read
    //WRITE: The file's data can be written
	enum class FileAccess : u8 {
		NONE, READ, WRITE, READ_WRITE
	};

	//!The flags of a file
	//IS_VIRTUAL: The file only exists in memory
	//IS_FOLDER: The file is a folder
    enum class FileFlags : u8 {

		NONE = 0x0,

        READ = 0x1,
        WRITE = 0x2,
		IS_VIRTUAL = 0x4,
		IS_FOLDER = 0x8,

		//TODO: If this file is allowed to make sub files or folders

		VIRTUAL_FILE = READ | IS_VIRTUAL,
		VIRTUAL_FILE_WRITE = VIRTUAL_FILE | WRITE,
		VIRTUAL_FOLDER = IS_FOLDER | READ | IS_VIRTUAL,
    };

	//!The types of file changes
	//ADD/DEL = user or program added file to / deleted file from the file system
	//UPDATE = user or program updated the contents of the file
	//MOVE = user or program renamed or moved the file
	enum class FileChange {
		DEL,
		ADD,
		UPDATE,
		MOVE
	};

	//!A handle to a file
	using FileHandle = u32;

	//!Max size of a file
	using FileSize = usz;

    //!The queried info about a file
    //The layout of a recursive file structure:
    //folders, files, recursive
    struct FileInfo {

		//friend class FileSystem;

		//!The difference between two file handles
		using SizeType = FileHandle;

		//!The full path in oic representation
		String path{};

		//!The file/folder name (can't contain any slashes)
		String name{};

        //!The last modification time of the file
        //When the file is virtual, this is set to 0
        time_t modificationTime{};

        //!Implementation specific data
        void *dataExt{};

		//!The total size of the file
		//When the file is a folder, this is set to 0
		FileSize fileSize{};

        //!The parent's file id
		FileHandle parent{};

        //!The start location of the folders
		FileHandle folderHint{};

        //!The start location of the files (fileHint - folderHint = folderCount)
		FileHandle fileHint{};

        //!The end location of the files (fileEnd - fileHint = fileCount)
		FileHandle fileEnd{};

        //!The flags of this file
        FileFlags flags{};

        //Helper functions

		SizeType getFolders() const;
		SizeType getFiles() const;
		SizeType getFileObjects() const;

		//!Determines if this file can be accessed with these flags
        bool hasAccess(FileAccess access) const;
        bool hasFlags(FileFlags flags) const;

		//!Determines if this file can be accessed with these flags
        static bool hasFlags(FileFlags target, FileFlags flags);

		bool isLocal() const;
		bool isVirtual() const;
		bool isFolder() const;
		bool hasData() const;
		bool hasRegion(FileSize size, FileSize offset) const;

    };

	class FileSystem;

    //!A callback for handling file changes and loops
    using FileCallback = void (*)(FileSystem*, const FileInfo&, void*);

    //!A callback for handling file changes and loops
    using FileChangeCallback = void (*)(FileSystem*, const FileInfo&, FileChange, void*);

	//!A virtual or physical file
	class File {

		friend class FileSystem;

	protected:

		FileSystem *fs;
		FileInfo f;
		bool isOpen{}, hasWritten{};

		File(FileSystem *fs, const FileInfo f): fs(fs), f(f) {}
		virtual ~File();

	public:

		virtual bool read(void *v, FileSize size, FileSize offset) const = 0;
		virtual bool write(const void *v, FileSize size, FileSize offset) = 0;

		virtual bool resize(FileSize size) = 0;

		inline bool hasRegion(FileSize size, FileSize offset) const { return f.hasRegion(size, offset); }

		inline const FileInfo &getFile() const { return f; }
		inline usz size() const { return f.fileSize; }
	};

    //!The class responsible for handling file I/O
    //A file system can also be implemented for an archive as well as a native file system
    //Every file system supports virtual files, though local and global files aren't always guaranteed
	//
    //~/ is the virtual file system (cached READ or READ_WRITE memory access)
    //./ is the local file system (non-cached READ_WRITE disk access)
    //../ and ./ get resolved to form the final path
    //\ is disallowed
    //This is called oic file notation
	//
	//Each file system has to handle keeping the file system up to date
	//
	//Note: Keep in mind that virtual FileInfo& is only valid while the FileSystem hasn't been resized (add/remove/move)
	//		and doesn't hold all values for local files (such as file/folder hints, parent)
	//		Use paths to avoid referencing invalid data; and lock & unlock the file system when reading or writing from it
	//
	class FileSystem {

	public:

		//!Initialize the file system
		//FileAccess::NONE can be used to disallow virtual files
		FileSystem(const FileAccess virtualFileAccess);

		//Constructors
		virtual ~FileSystem() = default;

		FileSystem(const FileSystem &) = delete;
		FileSystem &operator=(const FileSystem &) = delete;
		FileSystem(FileSystem &&) = delete;
		FileSystem &operator=(FileSystem &&) = delete;

		//!Open a file; has to be closed to allow it to be modified again
		//If you don't want to wait for the file to be available, set the max timeout to 0
		//	every retryTimeout it will attempt to open it again until maxTimeout is reached
		virtual File *open(const FileInfo &inf, ns maxTimeout = 500_ms, ns retryTimeout = 100_ms) = 0;

		//!Open a file by path
		inline File *open(const String &path, ns maxTimeout = 500_ms, ns retry = 100_ms) { return open(get(path), maxTimeout, retry); }

		//!Close a file; ensure it can be used again
		void close(File *f);

		//!Called to add a file modification callback at a directory
		void addFileChangeCallback(FileChangeCallback, const String &, void *);

		//!Called to remove a file modification callback
		void removeFileChangeCallback(const String&);

		//!Get the properties of a file
		//@param[in] path The target file object with oic file notation
		//@warning Throws if the file doesn't exist
		const FileInfo get(const String &path) const;

		//!Resolve the path to a normal file directory (without ../ and ./)
		//@param[in] path The target file object with oic file notation
		//@param[out] outPath
		//@return bool exists Whether the path leads to a valid path
		bool resolvePath(const String &path, String &outPath) const;

		//!Allows looping through the children of a folder
		bool foreachFile(const String &path, FileCallback callback, bool recurse, void*);

		//!Detect if the path exists
		//@param[in] path The target file object with oic file notation
		//@return bool exists Whether the path leads to a valid path
		bool exists(const String &path) const;

		//!Detect if the path exists
		//@param[in] path The target file object with oic file notation
		//@return bool exists Whether the file has the specified region
		bool regionExists(const String &path, FileSize size, FileSize offset) const;

		//!Read a (part of a) file into an address (means you have to allocate 'size' bytes)
		//@param[in] path The path in oic file notation
		//@param[out] address (u8[size])
		//@param[in] size The number of bytes to read (non-zero)
		//@param[in] offset The byte offset in the file
		//@return bool success
		bool read(const String &path, u8 *address, FileSize size, FileSize offset);

		//!Read a (part of a) file into a buffer
		//@param[in] path The path in oic file notation
		//@param[out] buffer The output
		//@param[in] size The number of bytes to read (0 = all by default)
		//@param[in] offset The byte offset in the file
		//@return bool success
		bool read(const String &file, Buffer &buffer, FileSize size = 0, FileSize offset = 0);

		//!Write to a (part of a) file from an address
		//@param[in] path The path in oic file notation
		//@param[out] address (u8[size])
		//@param[in] size The number of bytes to read (non-zero)
		//@param[in] offset The byte offset in the file
		//@return bool success
		bool write(const String &file, const u8 *address, FileSize size, FileSize offset);

		//!Write to a (part of a) file
		//@param[in] path The path in oic file notation
		//@param[out] buffer The input
		//@param[in] size The number of bytes to write (0 = all by default)
		//@param[in] bufferOffset The byte offset in the buffer
		//@param[in] fileOffset The byte offset in the file (0 = clear, usz_MAX = append, otherwise it tries to write into the offset)
		//@return bool success
		bool write(const String &file, const Buffer &buffer, FileSize size = 0, usz bufferOffset = 0, FileSize fileOffset = 0);

		//!Add a directory or file
		//@param[in] path The path in oic file notation
		//@param[in] isFolder If the file is capable of having children
		//@param bool isCallback; if this is true, it only invokes the callbacks
		//				this means it won't actually remove data
		//@return bool success
		bool add(const String &path, bool isFolder, bool isCallback = false);

		//!Remove a directory or file
		//@param[in] path The path in oic file notation
		//@param bool isCallback; if this is true, it only invokes the callbacks
		//				this means it won't actually remove data
		//@return bool success
		bool remove(const String &path, bool isCallback = false);

		//!Update the metadata for a file
		//@param[in] path The path in oic file notation
		//@return bool success
		bool update(const String &path);

		//!Move a file to a destination (and rename)
		//@param[in] path The path in oic file notation
		//@param[in] newPath The destination path in oic file notation
		//@param bool isCallback; if this is true, it only invokes the callbacks
		//				this means it won't actually remove data
		//@return bool success
		bool mov(const String &path, const String &newPath, bool isCallback = false);

		//Sizes of the file system

		inline FileHandle virtualSize() const { return FileHandle(virtualFiles.size()); }
		inline const List<FileInfo> &getVirtualFiles() const { return virtualFiles; }

		void lock();		//Wait for the file system to be available
		void unlock();		//Release the file system

		//Local access; not always present

		virtual const FileInfo local(const String &path) const = 0;
		virtual bool hasLocal(const String &path) const = 0;
		virtual bool hasLocalRegion(const String &path, FileSize size, FileSize offset) const = 0;

		virtual List<String> localDirectories(const String &path) const = 0;
		virtual List<String> localFileObjects(const String &path) const = 0;
		virtual List<String> localFiles(const String &path) const = 0;

    protected:

		//!Creates a local file
		virtual bool makeLocal(const String &, bool isFolder) = 0;

		//!Deletes a local file
		virtual bool delLocal(const String &) = 0;

		//!Creates the look up tables by file path
		void initLut();
    
        //!Called to initialize the file system cache
        virtual void initFiles() = 0;

		//!Used to handle file changes and update the metadata for the file
		virtual void onFileChange(const FileInfo &, FileChange) {}

		//!Start the watcher that updates the local file system
		//Called when a file change callback is created
		//should be handled in a different thread
		virtual void startFileWatcher(const String &path) = 0;

		//!End the watcher that updates the local file system
		//Called when a file change callback is removed
		virtual void endFileWatcher(const String &path) = 0;

		//!File cache
		List<FileInfo> virtualFiles;

    private:

		//!Helper function to obtain parts of the path (parses the ../ and ./ first)
		static usz obtainPath(const String &path, List<String> &splits);

		//!Rename file (no recursion)
		void rename(const FileInfo &info, const String &path, bool setName);

		//!File id by path look up tables
		HashMap<String, FileHandle> virtualFileLut;

        //!List of all file change callbacks
        HashMap<String, std::pair<FileChangeCallback, void*>> callbacks;

		std::mutex mutex;

    };

}