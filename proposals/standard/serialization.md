# Serialization

## Calling the serializer

```cpp
MyType myType;
Serializer<PrintSerializer>::serialize(myType);
```

## Adding serialization support

### Exposing the names

To make this work with a class, the class would have the following code:

```cpp
f32 x, y, z, w, h;

otc_serialize(0, x, y, z, w, h);
```

Where 0 is the version id.

Adding that to your class, where your members are x/y/z/w/h, it will work. Arithmetic types, containers and custom classes can all be supported.

### Not exposing the names

If the names aren't important, you can treat the object as a tuple:

```cpp
f32 x, y, z, w, h;

otc_serialize_tuple(0, x, y, z, w, h);
```

Where 0 is the version id.

This means that renaming your variables won't matter and it adds a layer of obfuscation.

### Versioning

`outdated<1, f32>` Will let the serializer know that since version 1, the float placed there won't be used. 

`updated<1, i32>(x)` Will let the serializer know that since version 1, there's an int placed there. 

```cpp
f32 x, y, z, w;
i32 h;

otc_serialize_tuple(1, x, y, z, w, outdated<1, f32>, updated<1, i32>(h))
```

## Custom serializer

Reflection is done through serializer objects; this means that you can declare your own templated struct:

```cpp
template<typename T, bool inObject>
struct PrintSerializer {

	//Data required for this serialization call
	PrintSerializer(){}

	//Called on primary data types (like float/int/string)
	inline void serialize(const c8 *x, const usz elementOffset, T &t) {
		if constexpr(inObject)
	        std::cout << x << ":" << t << std::endl;
	    else
	        std::cout << elementOffset << ":" << t << std::endl;
	}
    
	//Called on all iterables
	inline void serializeArray(const c8 *x, const usz elementOffset){
	    if constexpr(inObject)
	    	std::cout << "Array at " << x;
	    else
	        std::cout << "Array at " << elementOffset;
	}
    
	//Called on all objects
	inline void serializeObject(const c8 *x, const usz elementOffset){
	    if constexpr(inObject)
	    	std::cout << "Object at " << x;
	    else
	       	std::cout << "Object at " << elementOffset;
	}

	//Called after types
	inline void serializeEnd(){ /* content */ }
	inline void serializeArrayEnd(){ /* content */ }
	inline void serializeObjectEnd(){ /* content */ }

};
```

## Serialized example

```cpp
[ "test", "hello" ], { "a": [ 1, 2, 3 ], "b": 5, "def": -12 }, 1.5
```

In the example above, it will do the following calls (if those functions exist):

```cpp
//"[ "
JSONSerializer<T, false>::serializeArray(nullptr, 0);

//""test""
JSONSerializer<c8*, false>::serialize(nullptr, 0, "test");

//", " 
JSONSerializer<T, false>::serializeEnd();

//""hello""
JSONSerializer<c8*, false>::serialize(nullptr, 1, "hello");

//" ]"
JSONSerializer<T, false>::serializeArrayEnd();

//", "
JSONSerializer<T, false>::serializeEnd();

//"{ "
JSONSerializer<T, false>::serializeObject(nullptr, 1);

//""a": [ "
JSONSerializer<T, true>::serializeArray("a", -1);

//"1"
JSONSerializer<f64, false>::serialize(nullptr, 0, 1);

//", "
JSONSerializer<T, false>::serializeEnd();

//"2"
JSONSerializer<f64, false>::serialize(nullptr, 0, 2);

//", "
JSONSerializer<T, false>::serializeEnd();

//"3"
JSONSerializer<f64, false>::serialize(nullptr, 0, 3);

//" ]"
JSONSerializer<T, false>::serializeArrayEnd();

//", "
JSONSerializer<T, true>::serializeEnd();

//""b": 5"
JSONSerializer<u32, true>::serialize("b", -1, 5);

//", "
JSONSerializer<T, true>::serializeEnd();

//""def": -12"
JSONSerializer<i32, true>::serialize("def", -1, -12);

//" }"
JSONSerializer<T, false>::serializeObjectEnd();

//", "
JSONSerializer<T, false>::serializeEnd();

//"1.5"
JSONSerializer<f64, false>::serialize(nullptr, 2, 1.5);
```

"serialize" is called on all low-level objects, this means: arithmetic types (`u<x>`/`i<x>`/bool/`c<x>`) and strings.

"serializeArray" is called on all iterables/containers; like maps, lists/arrays, bitsets, etc. This doesn't give you access to the array, but only lets you know it exists and what the element id / member name is. This is because "serialize" takes care of the contents of the array. 

"serializeObject" is the same as serializeArray but is only called on objects.

For every serialize function, there's an end function. "serializeArrayEnd/serializeObjectEnd" are always called after an object or array was finished. "serializeEnd" is called after the object has serialized **_If there is a next object_**. 
