#pragma once

#include "./../execution/bytecode.hpp"
#include "./function.hpp"

#include <iostream>
#include <vector>
#include <string>


class Table{
	public:
		Table(std::vector<RawAction> tokens);

		Function* Find(std::string str);
		bool valid;
	private:
		std::vector<Function> function;
};


#include "./table.cpp"
