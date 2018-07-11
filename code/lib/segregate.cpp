#include "./segregate.hpp"

namespace Segregate{
	bool IsParamBreak(char val){
		if (val == ' '){
			return true;
		}else if (val == '\t'){
			return true;
		}

		return false;
	};
	bool IsLineBreak(char val){
		if (val == '\n'){
			return true;
		}

		return false;
	};

	std::string RemoveCarrageReturn(std::string str){
		unsigned long length = str.size();
		for (unsigned long i=0; i<length; i++){
			if (str[i] == '\r'){
				// Left shift all chars
				for (unsigned long j=i+1; j<length; j++){
					str[j-1] = str[j];
				}

				// Store length due to pop
				length--;
			}
		}

		// Change string length to ignore trailing invalid chars
		str.resize(length);

		return str;
	}

	// Splits a single line into individual parameters
	ParamArray Parameterize(std::string str){
		str.assign( RemoveCarrageReturn(str) );
		str.append(" "); // Ensures that the loop will cover all segments
		unsigned long length = str.size();
		unsigned long comStart = 0;
		ParamArray res;


		// Ignore indentation
		//  Find the start of actual content
		while (comStart < length && IsParamBreak(str[comStart])){
			comStart++;
		}


		// Count the number of parameters
		unsigned long vecLen = 0;
		for (unsigned long i=comStart; i<length; i++){
			if ( IsParamBreak(str[i]) ){
				vecLen++;
			}
		}
		res.resize(vecLen);  // Initilize veclen objects


		// Scan will fail if there is no data
		if (length == 0){
			return res;
		}


		// Slice the the string into parts
		unsigned long toIndex = 0;
		unsigned long cursor = comStart;
		for (unsigned long i=cursor; i<length; i++){
			if (IsParamBreak(str[i]) == true){
				// Transfer the sub-string to the vector,
				//  Ensure that the data is it's own, and not a reference
				res[toIndex].assign( str.substr(cursor, i-cursor) );

				cursor = i+1;
				toIndex++;
			}
		}

		return res;
	};


	StrCommands Fragment(std::string str){
		str.append("\n"); // Ensures that the loop will cover all segments
		unsigned long length = str.size();

		// Result
		StrCommands res;


		// Count lines
		//  Ignoring empty lines
		unsigned long vecLen = 0;
		for (unsigned long i=0; i<length; i++){
			if (IsLineBreak(str[i])){
				vecLen++;
			}
		}
		res.resize(vecLen);


		// Ignore 'empty' strings as they may cause errors
		if (vecLen == 0){
			return res;
		}


		// Read lines
		unsigned long toIndex = 0;
		unsigned long cursor = 0;
		unsigned long line = 1;
		for (unsigned long i=0; i<length; i++){
			if (IsLineBreak(str[i])){
				res[toIndex].param = Parameterize( str.substr(cursor, i-cursor));
				res[toIndex].line = line;

				// Ignore blank lines
				if (res[toIndex].param.size() == 0){
					vecLen--;
				}else{
					toIndex++;
				}
				cursor = i+1;
				line++;
			}
		}


		// Shrink the result due to undersizing for blank lines
		res.resize(vecLen);

		return res;
	};
}
