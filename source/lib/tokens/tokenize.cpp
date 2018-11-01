#include "./tokenize.hpp"

namespace Tokenize{
	// Remove empty elements of an array in one pass
	std::vector< std::vector<std::string> > RemoveEmpty(std::vector< std::vector<std::string> > arr){
		unsigned long size   = arr.size();
		unsigned long offset = 0;
		
		for (unsigned long i=0; i<size; i++){
			// This element is empty, increase the shift amount to cover it
			if (arr[i].size() == 0){
				offset ++;
				continue;
			}
			
			// Shift all items back i slots
			if (offset > 0){
				arr[i-offset] = arr[i];
			}
		};
		
		arr.resize(size - offset);
		
		return arr;
	};
	
	std::vector<std::string> SplitParameters(std::string str){
		str += " "; // Add a trailing space to ensure no final characters are missed
		
		std::vector<std::string> out;
		unsigned long size = str.size();
		unsigned long len = 0;
		unsigned long j = 0;
		unsigned long i = 0;
		
		for (; i<size; i++){
			if (str[i] == ' ' || str[i] == '\t' || str[i] == '\r'){
				len = i-j;
				
				if (len < 1){
					j = i+1;
					continue;
				}
				
				out.push_back(str.substr(j, len));
				j = i+1;
			}
		}
		
		return out;
	};
	
	std::vector< std::vector<std::string> > SplitLines(std::string str){
		str += "\n"; // Add a trailing space to ensure no final characters are missed
		
		std::vector< std::vector<std::string> > out;
		unsigned long size = str.size();
		unsigned long len = 0;
		unsigned long j = 0;
		unsigned long i = 0;
		
		for (; i<size; i++){
			if (str[i] == '\n'){
				len = i-j;
				
				if (len < 1){
					j = i+1;
					continue;
				}
				
				out.push_back( SplitParameters(str.substr(j, len)) );
				j = i+1;
			}
		}
		
		out = RemoveEmpty(out);
		return out;
	};
}
