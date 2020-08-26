#pragma once
#include "types/types.hpp"

namespace oic {

	struct ArgHelper {

		static List<String> makeNames(const String &name) {

			List<String> args;
			args.reserve(std::count(name.begin(), name.end(), ','));

			size_t scopes[4]{};
			bool inQuotes[2]{};

			usz lastOccurrence = usz_MAX, end = usz_MAX;
			bool containsNumbers{}, containsLetters{}, isValid = true;

			for (usz i{}, j = name.size(); i < j; ++i) {

				c8 c = name[i];

				switch (c) {

					case ' ': case '\t': case '\r': case '\n': 
						break;

					case '{':	++scopes[0];					isValid = false;	break;
					case '}':	--scopes[0];					isValid = false;	break;
					case '<':	++scopes[1];					isValid = false;	break;
					case '>':	--scopes[1];					isValid = false;	break;
					case '[':	++scopes[2];					isValid = false;	break;
					case ']':	--scopes[2];					isValid = false;	break;
					case '(':	++scopes[3];					isValid = false;	break;
					case ')':	--scopes[3];					isValid = false;	break;

					//Quotes

					case '"':	inQuotes[0] = !inQuotes[0];		isValid = false;	break;

					case '\'':

						isValid = false;

						if (!containsNumbers)				//Numbers only is like 1'000
							inQuotes[1] = !inQuotes[1];

						break;

					//Split commas

					case ',':

						if (
							!scopes[0] && !scopes[1] && 
							!scopes[2] && !scopes[3] && 
							!inQuotes[0] && !inQuotes[1]
						) {
							containsNumbers = containsLetters = false;

							if (isValid && lastOccurrence != usz_MAX) {
								String str = name.substr(lastOccurrence, end - lastOccurrence);
								std::replace(str.begin(), str.end(), '_', ' ');
								args.push_back(str);
								lastOccurrence = end = usz_MAX;
							}

							isValid = true;
						}

						break;

					default:

						if (!isValid) break;

						if (c >= '0' && c <= '9') {

							if (!containsLetters)
								containsNumbers = true;

							else end = i + 1;
						}

						else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {

							containsLetters = true;

							if (lastOccurrence == usz_MAX)
								lastOccurrence = i;
							
							end = i + 1;
						}
				}

			}

			if (isValid && lastOccurrence != usz_MAX) {
				String str = name.substr(lastOccurrence, end - lastOccurrence);
				std::replace(str.begin(), str.end(), '_', ' ');
				args.push_back(str);
			}

			return args;
		}

	};

}

//Either reflection or inspection; inflection

//InflectBody(inflector.inflect(this, recursion, { "My name", "My test" }, myName, myTest);

#define InflectBody(...)																			\
template<typename T, typename T2>																	\
void inflect(T &inflector, usz recursion, const T2*) {												\
	__VA_ARGS__																						\
}																									\
																									\
template<typename T, typename T2>																	\
void inflect(T &inflector, usz recursion, const T2*) const {										\
	__VA_ARGS__																						\
}
#define InflectBodyWithParent(...)																	\
template<typename T, typename T2>																	\
void inflect(T &inflector, usz recursion, const T2 *parent) {										\
	__VA_ARGS__																						\
}																									\
																									\
template<typename T, typename T2>																	\
void inflect(T &inflector, usz recursion, const T2 *parent) const {									\
	__VA_ARGS__																						\
}

//Inflect(Test, My_name_with_spaces);

#define Inflect(...) InflectBody(																	\
	static const List<String> namesOfArgs = oic::ArgHelper::makeNames(#__VA_ARGS__);				\
	inflector.inflect(this, recursion, namesOfArgs, __VA_ARGS__);									\
);
//InflectParented(Parent, Test, My_name_with_spaces);

#define InflectParented(Parent, ...) InflectBodyWithParent(											\
	static const List<String> namesOfArgs = oic::ArgHelper::makeNames(#__VA_ARGS__);				\
	Parent::inflect(inflector, recursion + 1, parent);												\
	inflector.inflect(this, recursion, namesOfArgs, __VA_ARGS__);									\
);

//InflectWithName({ "Test, "Test2" }, test, test2);

#define InflectWithName(...) InflectBody(															\
	inflector.inflect(this, recursion, __VA_ARGS__);												\
);

//InflectParentedWithName(Parent, { "Test, "Test2" }, test, test2);

#define InflectParentedWithName(Parent, ...) InflectBodyWithParent(									\
	Parent::inflect(inflector, recursion + 1, parent);												\
	inflector.inflect(this, recursion, __VA_ARGS__);												\
);