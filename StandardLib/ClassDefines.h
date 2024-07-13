#ifndef __CLASS_DEFINES_H__
#define __CLASS_DEFINES_H__


#define M_Pointer(name) Pointer(name.This(), #name)
#define M_Embedded(name) Embedded(&name, #name);

#define M_Int8(name)		Primitive(&name, #name)
#define M_UInt8(name)		Primitive(&name, #name)
#define M_Int16(name)		Primitive(&name, #name)
#define M_UInt16(name)		Primitive(&name, #name)
#define M_Int32(name)		Primitive(&name, #name)
#define M_UInt32(name)		Primitive(&name, #name)
#define M_Int64(name)		Primitive(&name, #name)
#define M_UInt64(name)		Primitive(&name, #name)
#define M_Bool(name)		Primitive(&name, #name)
#define M_Float32(name)		Primitive(&name, #name)
#define M_Float64(name)		Primitive(&name, #name)
#define M_Char8(name)		Primitive(&name, #name)
#define M_Char16(name)		Primitive(&name, #name)


#define U_Int8(name)					UnmanagedInt8(&name, #name)
#define U_Int8Array(name, length)		UnmanagedInt8(name, length, #name)
#define U_Int16(name)					UnmanagedInt16(&name, #name)
#define U_Int16Array(name, length)		UnmanagedInt16(name, length, #name)
#define U_Int32(name)					UnmanagedInt32(&name, #name)
#define U_Int32Array(name, length)		UnmanagedInt32(name, length, #name)
#define U_Int64(name)					UnmanagedInt64(&name, #name)
#define U_Int64Array(name, length)		UnmanagedInt64(name, length, #name)
#define U_SInt(name)					UnmanagedSInt(&name, #name)
#define U_SIntArray(name, length)		UnmanagedSInt(name, length, #name)
#define U_UInt8(name)					UnmanagedUInt8(&name, #name)
#define U_UInt8Array(name, length)		UnmanagedUInt8(name, length, #name)
#define U_UInt16(name)					UnmanagedUInt16(&name, #name)
#define U_UInt16Array(name, length)		UnmanagedUInt16(name, length, #name)
#define U_UInt32(name)					UnmanagedUInt32(&name, #name)
#define U_UInt32Array(name, length)		UnmanagedUInt32(name, length, #name)
#define U_UInt64(name)					UnmanagedUInt64(&name, #name)
#define U_UInt64Array(name, length)		UnmanagedUInt64(name, length, #name)
#define U_SUInt(name)					UnmanagedSUInt(&name, #name)
#define U_SUIntArray(name, length)		UnmanagedSUInt(name, length, #name)
#define U_Size(name)					UnmanagedSize(&name, #name)
#define U_SizeArray(name, length)		UnmanagedSize(name, length, #name)
#define U_Float32(name)					UnmanagedFloat(&name, #name)
#define U_Float32Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Float64(name)					UnmanagedFloat(&name, #name)
#define U_Float64Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Float96(name)					UnmanagedFloat(&name, #name)
#define U_Float96Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Char(name)					UnmanagedChar(&name, #name)
#define U_CharArray(name, length)		UnmanagedChar(name, length, #name)
#define U_Char16(name)					UnmanagedChar(&name, #name)
#define U_Char16Array(name, length)		UnmanagedChar(name, length, #name)
#define U_Bool(name)					UnmanagedBool(&name, #name)
#define U_BoolArray(name, length)		UnmanagedBool(name, length, #name)

#define U_Enum(name)					UnmanagedEnum(&name, #name)

#define U_String(name)					UnmanagedString(&name, #name)
#define U_Number(name)					UnmanagedNumber(&name, #name)
#define U_Date(name)					UnmanagedDate(&name, #name)
				
#define U_Data(type, name)				UnmanagedData(&name, sizeof(type), #name)
#define U_Unknown(type, name)			UnmanagedUnknown(&name, sizeof(type), #name)
#define U_Pointer(name)					UnmanagedPointer((void**)&name, #name)


#define U_2Float32(name)				Unmanaged2Float(&name, #name)
#define U_2Float32Array(name, length)	Unmanaged2Float(name, length, #name)
#define U_3Float32(name)				Unmanaged3Float(&name, #name)
#define U_3Float32Array(name, length)	Unmanaged3Float(name, length, #name)
#define U_4Float32(name)				Unmanaged4Float(&name, #name)
#define U_4Float32Array(name, length)	Unmanaged4Float(name, length, #name)
#define U_4x4Float32(name)				Unmanaged4x4Float(&name, #name)
#define U_4x4Float32Array(name, length)	Unmanaged4x4Float(name, length, #name)


#endif // __CLASS_DEFINES_H__

