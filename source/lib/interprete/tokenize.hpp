#pragma once

#include "./../typebase.hpp"

#include <string>
#include <vector>


class RawAction{
	public:
		std::vector<std::string> param;
		uint64 line;
};

std::string ToString(RawAction act);

#include "./tokenize.cpp"
