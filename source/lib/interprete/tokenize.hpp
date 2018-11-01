#pragma once

#include "./../typebase.hpp"

#include <string>
#include <vector>


class RawAction{
	public:
		std::vector<std::string> param;
		uint64 line;
};

#include "./tokenize.cpp"
