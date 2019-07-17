#pragma once
#include "types/types.hpp"

namespace oic::errors {

	//!Thrown by file_system.cpp and children
	namespace fs {

		//!Occurs when the user uses a file path that can't be resolved (global files, including backslashes or escaping file system)
		static constexpr c8 invalid[] = "The file path is invalid";

		//!Occurs when the local file system implementation is out of memory
		static constexpr c8 outOfMemory[] = "The file system is out of memory";

		//!Occurs when the file or folder cannot be found in the file system
		static constexpr c8 nonExistent[] = "The requested file object doesn't exist";

		//!Occurs when the operation is not supported by the file system
		//For example if the file system doesn't allow local files and one is requested
		static constexpr c8 notSupported[] = "The requested operation is not supported by the file system";

		//!Occurs when the operation is out of bounds
		//E.g. reading or writing outside of the file's address space
		static constexpr c8 outOfBounds[] = "The requested operation was out of bounds";

		//!Occurs when the operation is not allowed logically
		//E.g. deleting the root folder in a file system
		static constexpr c8 illegal[] = "The requested operation is illegal";

	}

	//!Thrown by system.cpp and children
	namespace sys {

		//!Called if multiple instances of the system class exist
		static constexpr c8 alreadyExists[] = "There can only be one system";

	}

	//!Thrown by log's children on fatal errors
	namespace sig {

		static constexpr c8 abrt[] = "System was aborted";
		static constexpr c8 fpe[] = "Fatal floating point error";
		static constexpr c8 ill[] = "Illegal system instruction";
		static constexpr c8 interupt[] = "Interrupt signal was triggered";
		static constexpr c8 segv[] = "Segmentation fault; accessing invalid memory";
		static constexpr c8 term[] = "System was terminated";
		static constexpr c8 undef[] = "Undefined exception has occurred";

	}

	//!Thrown by types
	namespace typ {

		static constexpr c8 outOfBounds[] = "Operation was out of bounds";

	}

}