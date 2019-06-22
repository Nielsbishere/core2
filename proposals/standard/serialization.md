# Serialization

## Calling the serializer

```cpp
JSONWriter jsonSerialize = 
    otc::Serializer::serialize<JSONWriter>(...);
```

## Adding serialization support

### Exposing the names

To make this work with a class, the class would have the following code:

```cpp
f32 x, y, z, w, h;

otc_serialize(0, x, y, z, w, h);
```

Where 0 is the version id. This will expose the member names to the serializer; though serializers can still ignore this if they don't need it.

### Not exposing the names

If the names aren't important, you can treat the object as a tuple:

```cpp
f32 x, y, z, w, h;

otc_serialize_tuple(0, x, y, z, w, h);
```

Where 0 is the version id.

This means that renaming your variables won't matter and it adds a layer of obfuscation.

### TODO: Versioning

`otc::outdated<1, f32>` Will let the serializer know that since version 1, the float placed there won't be used. 

`otc::updated<1, i32>(x)` Will let the serializer know that since version 1, there's an int placed there. 

```cpp
f32 x, y, z, w;
otc::outdated<1, f32> h_;	//Outdated h (1 byte in memory, 4 or 0 on disk)
otc::updated<1, i32> h;		//Updated h (4 bytes)

otc_serialize_tuple(1, x, y, z, w, h_, h);
```

### TODO: Sections

`otc::section<T>` Will let the serializer know that if the 'sections' variable is big enough, it will serialize this type. Example;

```cpp
u16 sections;
otc::section<f32> someFloat, someOtherFloat;

otc_serialize_tuple_sections(1, sections, someFloat, someOtherFloat);
```

If sections is 0, there won't be any float. If sections is 1, the last float won't be present.

If the struct that uses `otc::section` doesn't have the sections unsigned sized integer, it will assume it doesn't need to serialize. 

Usage of these sections is as follows:

```cpp
//Obtain variable and safety check
f32 *someFloat = myType.section<f32>();
f32 *someOtherFloat = myType.section<f32, 1>();
if(type)
    ;	//Do stuff with our float
```

When allocating a new struct, it will be the maximum size of the struct; however, in memory or on disk it can be a different size. If you cast memory to a `T*` for binary serialization, it might not have all sections that it could have. The section function takes care of this by checking the number of sections; it returns nullptr if you don't have these sections.

The id passed into the section function is the local index; putting a `otc::section<i32> someTest` between someFloat and someOtherFloat won't change how you access someOtherFloat through `.section<f32, 1>`. Though inserting another float before it will. Without the type as the first argument, it will pick the type of the global index given.

**Accessing sections directly through the member is a bad idea, as there is no guarantee of it being present in memory.**

### TODO: Inheritance

## Custom serializer

Reflection is done through serializer objects; this means that you can declare your own templated struct. Serialization can have a 'Serialsize' step to index through all types and reserve space for them. This can be done by simply `using Serialsize = T;` in the Serialize struct.

```cpp
struct JSONWriter {

    //Preprocess the types by running them through a size checker
	using Serialsize = JSONSerialsize;

    //Data used for serializing
	std::string result;
	usz offset{}, size;
    
    //Initializes all data used for serializing
	JSONWriter(JSONSerialsize &totalSize): 
    	size(totalSize.size), result(totalSize.size, '0') { }

    //Called on (u)ints/floats/chars/strings
    //inObject: if T is a field of an object
    //count != 0: the type is a C-style array
    //T: u<x>/i<x>/f<x>/c<x>/bool or const c<x>*
	template<bool inObject, usz count, typename T>
	inline void serialize(const c8 *member, T &t){}
    
	//Called on all iterables
    //inObject: if T is a field of an object
    //T: any class that has a begin(), end() and usz size() function.
	template<bool inObject, typename T>
	inline void serializeArray(const c8 *member, T &t) {}
    
	//Called on all objects
    //inObject: if T is a field of an object
    //!isTuple: the type has a name in the object
	template<bool inObject, bool isTuple>
	inline void serializeObject(const c8 *member){}

	//Called in between types in arrays / fields
	inline void serializeEnd(){ }
    
    //Called after iterables (lists, maps, etc.)
	inline void serializeIterableEnd(){ /* content */ }
    
    //Called after objects
    //!isTuple: the type has a name in the object
	template<bool isTuple>
	inline void serializeObjectEnd(){ /* content */ }

};
```

TODO: If your serializer is a binary serializer, the serialize function won't be called for arithmetic or plain old data structs. This means that you have to output this yourself in serializeArray, to increase performance.

## TODO: Errors

The serializer will produce errors if it finds a type it deems non-serializable. This means it's not iterable, not arithmetic, doesn't have a serialize function or it is defined as an invalid type. 

The following types are marked invalid:

| Type             | Error code                                                   | Reason                                                       |
| ---------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| usz / size_t     | Serialized field contains architecture depending variable.   | usz is 32 or 64 bit depending on the architecture; resulting in issues with serialization. |
| T*               | Serialized field is a pointer, this is not supported.        | Pointers are a problem for serialization, as the type could be virtual, non-serializable or an array. A void pointer is not serializable as well. |
| function pointer | Serialized field is a function pointer, this is not supported. | Function pointers exposed to the serializer are a security vulnerability; it could be abused to cause remote code execution. |
| long double      | Serialized field is a long double, this is not supported.    | Long doubles have varying sizes depending on the compiler and architecture; they aren't standardized. |

To allow serialization of virtual inherited objects, you can use modern pointers (std::shared_ptr). This only works with virtual serialize functions.