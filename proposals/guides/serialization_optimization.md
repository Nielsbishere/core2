# Serialization optimization

Optimizing storage with serialization can be done in the following ways:

- All - Removing the member names from the struct
- Binary - Reducing the in-memory size of the object
- Binary - Reducing array size bytes
- All - Reducing the in-storage size (compression)

## Removing member names

Member names take up space, they are serialized too. This means that removing them can reduce the total size of the serialized object. 

This can be done by switching your 'otc_serialize' to 'otc_serialize_tuple'; which doesn't expose your variable names. 

## Reducing struct size

This can be done by using less bits per variable or using a union with bitflags;

```cpp
union {
  
    u8 data_;
    
    struct {
    	u8 first : 2;	//& 0b00000011
     	u8 second : 4;	//& 0b00111100
        u8 third : 2;	//& 0b11000000
    };
    
};
```

And then passing data_ to the serializer. This can also be reducing your data types to a short instead of a full int.

## Reducing array size bytes

An array/container specifies how many elements are in it in binary serializers. This means that it requires 8 additional bytes for every string/array. To reduce this, you can use the otc::Array type, which allows you to specify what type it uses for size (u8, u16, u32, u64, void (no serialization of the array size; so you can handle it yourself)). 

## Compression

If you want a specialized compression/decompression technique, you can create an intermediate and call serialize on that. You can determine whether or not it's serializing/deserializing so you can determine when to compress and when to decompress.