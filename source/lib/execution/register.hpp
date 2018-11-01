#pragma once
#include "./../typebase.hpp"


#define RegisterCount 9

/*
	The 'active' variables within the system.
	These can either behave based on the neureric value they point to
	or the address it's self.
	@rA => using address
	&rA => using refered value

	Datatypes:
		- (u)int8  => (un)signed char
		- (u)int16 => (un)signed short
		- (u)int32 => (un)signed long
		- (u)int64 => (un)signed long long
		-  float32 => float
		-  float64 => double
*/


union Handle{
	// Address
	void* address;

	// Integer values
	uint8  uint8;
	 int8   int8;
	uint16 uint16;
	 int16  int16;
	uint32 uint32;
	 int32  int32;
	uint64 uint64;
	 int64  int64;

	// Float values
	float32 float32;
	float64 float64;
};
enum class RegisterMode{
	invalid,

	uint8,  int8,
	uint16, int16,
	uint32, int32,
	uint64, int64,

	float32,
	float64,
};



#include "./register.cpp"
