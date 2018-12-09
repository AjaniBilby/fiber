#pragma once

#include <iostream>
#include <cstdint>

typedef double float64;
typedef float  float32;

typedef uint64_t uint64;
typedef  int64_t int64;
typedef uint32_t uint32;
typedef  int32_t int32;
typedef uint16_t uint16;
typedef  int16_t int16;
typedef uint8_t  uint8;
typedef  int8_t  int8;

typedef void* address;


bool CheckTypeSizing(){
	bool valid = true;

	// Check ints
	if (sizeof(int64) != 8){
		std::cerr << "Invalid (u)int64 size" << std::endl;
		std::cerr << "  " << sizeof(int64) << "!= 8" << std::endl;
		valid = false;
	};
	if (sizeof(int32) != 4){
		std::cerr << "Invalid (u)int32 size" << std::endl;
		std::cerr << "  " << sizeof(int32) << "!= 4" << std::endl;
		valid = false;
	};
	if (sizeof(int16) != 2){
		std::cerr << "Invalid (u)int16 size" << std::endl;
		std::cerr << "  " << sizeof(int16) << "!= 2" << std::endl;
		valid = false;
	};
	if (sizeof(int8) != 1){
		std::cerr << "Invalid (u)int8 size" << std::endl;
		std::cerr << "  " << sizeof(int8) << "!= 1" << std::endl;
		valid = false;
	};

	// Check floats
	if (sizeof(float64) != 8){
		std::cerr << "Invalid float64 size" << std::endl;
		std::cerr << "  " << sizeof(float64) << "!= 8" << std::endl;
		valid = false;
	};
	if (sizeof(float32) != 4){
		std::cerr << "Invalid float32 size" << std::endl;
		std::cerr << "  " << sizeof(float32) << "!= 4" << std::endl;
		valid = false;
	};

	// Check adress
	if (sizeof(address) < 4){
		std::cerr << "Invalid address size" << std::endl;
		std::cerr << "  " << sizeof(address) << "< 4" << std::endl;
		valid = false;
	};

	return valid;
};
