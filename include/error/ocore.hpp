#pragma once
#include "types/types.hpp"

namespace oic::errors {

	//!Thrown by file_system.cpp and children
	namespace fs {

		//!Occurs when the user uses a file path that can't be resolved (global files, including backslashes or escaping file system)
		static constexpr c8 invalid[] = "The file path is invalid";

		//!Occurs when the file or folder cannot be found in the file system
		static constexpr c8 nonExistent[] = "The requested file object doesn't exist";

		//!Occurs when the file path is not supported by the file system
		//For example if the file system doesn't allow local files and one is requested
		static constexpr c8 notSupported[] = "The requested file object is not supported by the file system";

	}

}