#include "./register.hpp"


/*
	A Register is a reference to a specific value and it's type
*/
class Register{
	public:
		// Define the data type
		bool      isInt; // int : float
		bool   isSigned; // signed : unsigned
		size_t    bytes;

	public:
		Handle *pointer;

		Register(){
			this->isInt    = true;
			this->isSigned = false;
			this->pointer  = nullptr;
			this->bytes    = 0;
		};


		// Iterate on the existing value
		template <typename T>
		Register& operator+=(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 += value;
							break;
						case 2:
							this->pointer->int16 += value;
							break;
						case 4:
							this->pointer->int32 += value;
							break;
						case 8:
							this->pointer->int64 += value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 += value;
							break;
						case 2:
							this->pointer->uint16 += value;
							break;
						case 4:
							this->pointer->uint32 += value;
							break;
						case 8:
							this->pointer->uint64 += value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 += value;
				}else{
					this->pointer->float32 += value;
				}
			}

			return *this;
		};
		template <typename T>
		Register& operator-=(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 -= value;
							break;
						case 2:
							this->pointer->int16 -= value;
							break;
						case 4:
							this->pointer->int32 -= value;
							break;
						case 8:
							this->pointer->int64 -= value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 -= value;
							break;
						case 2:
							this->pointer->uint16 -= value;
							break;
						case 4:
							this->pointer->uint32 -= value;
							break;
						case 8:
							this->pointer->uint64 -= value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 -= value;
				}else{
					this->pointer->float32 -= value;
				}
			}

			return *this;
		};
		template <typename T>
		Register& operator*=(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 *= value;
							break;
						case 2:
							this->pointer->int16 *= value;
							break;
						case 4:
							this->pointer->int32 *= value;
							break;
						case 8:
							this->pointer->int64 *= value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 *= value;
							break;
						case 2:
							this->pointer->uint16 *= value;
							break;
						case 4:
							this->pointer->uint32 *= value;
							break;
						case 8:
							this->pointer->uint64 *= value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 *= value;
				}else{
					this->pointer->float32 *= value;
				}
			}

			return *this;
		};
		template <typename T>
		Register& operator/=(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 /= value;
							break;
						case 2:
							this->pointer->int16 /= value;
							break;
						case 4:
							this->pointer->int32 /= value;
							break;
						case 8:
							this->pointer->int64 /= value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 /= value;
							break;
						case 2:
							this->pointer->uint16 /= value;
							break;
						case 4:
							this->pointer->uint32 /= value;
							break;
						case 8:
							this->pointer->uint64 /= value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 /= value;
				}else{
					this->pointer->float32 /= value;
				}
			}

			return *this;
		};
		template <typename T>
		Register& operator%=(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 %= value;
							break;
						case 2:
							this->pointer->int16 %= value;
							break;
						case 4:
							this->pointer->int32 %= value;
							break;
						case 8:
							this->pointer->int64 %= value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 %= value;
							break;
						case 2:
							this->pointer->uint16 %= value;
							break;
						case 4:
							this->pointer->uint32 %= value;
							break;
						case 8:
							this->pointer->uint64 %= value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 %= value;
				}else{
					this->pointer->float32 %= value;
				}
			}

			return *this;
		};

		void set(Register* value){
			size_t size;
			if (this->bytes < value->bytes){
				size = this->bytes;
			}else{
				size = value->bytes;
			}

			if (size == 8){
				this->pointer->uint64 = value->pointer->uint64;
			}else if (size == 4){
				this->pointer->uint32 = value->pointer->uint32;
			}else if (size == 2){
				this->pointer->uint16 = value->pointer->uint16;
			}else if (size == 1){
				this->pointer->uint32 = value->pointer->uint32;
			}
		}

		// Set value to
		template <typename T>
		void set(T value){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							this->pointer->int8 = value;
							break;
						case 2:
							this->pointer->int16 = value;
							break;
						case 4:
							this->pointer->int32 = value;
							break;
						case 8:
							this->pointer->int64 = value;
							break;
					}
				}else{
					switch (this->bytes){
						case 1:
							this->pointer->uint8 = value;
							break;
						case 2:
							this->pointer->uint16 = value;
							break;
						case 4:
							this->pointer->uint32 = value;
							break;
						case 8:
							this->pointer->uint64 = value;
							break;
					}
				}
			}else{
				if (this->bytes == 8){
					this->pointer->float64 = value;
				}else{
					this->pointer->float32 = value;
				}
			}
		};


		// Get the referenced information in a specific form
		uint64 toUint64(){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							return this->pointer->int8;
						case 2:
							return this->pointer->int16;
						case 4:
							return this->pointer->int32;
						case 8:
							return this->pointer->int64;
					}
				}else{
					switch (this->bytes){
						case 1:
							return this->pointer->uint8;
						case 2:
							return this->pointer->uint16;
						case 4:
							return this->pointer->uint32;
						case 8:
							return this->pointer->uint64;
					}
				}
			}else{
				if (this->bytes == 8){
					return this->pointer->float64;
				}else{
					return this->pointer->float32;
				}
			}

			return 0;
		}
		int64 toInt64(){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							return this->pointer->int8;
						case 2:
							return this->pointer->int16;
						case 4:
							return this->pointer->int32;
						case 8:
							return this->pointer->int64;
					}
				}else{
					switch (this->bytes){
						case 1:
							return this->pointer->uint8;
						case 2:
							return this->pointer->uint16;
						case 4:
							return this->pointer->uint32;
						case 8:
							return this->pointer->uint64;
					}
				}
			}else{
				if (this->bytes == 8){
					return this->pointer->float64;
				}else{
					return this->pointer->float32;
				}
			}

			return 0;
		};
		float32 toFloat32(){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							return this->pointer->int8;
						case 2:
							return this->pointer->int16;
						case 4:
							return this->pointer->int32;
						case 8:
							return this->pointer->int64;
					}
				}else{
					switch (this->bytes){
						case 1:
							return this->pointer->uint8;
						case 2:
							return this->pointer->uint16;
						case 4:
							return this->pointer->uint32;
						case 8:
							return this->pointer->uint64;
					}
				}
			}else{
				if (this->bytes == 8){
					return this->pointer->float64;
				}else{
					return this->pointer->float32;
				}
			}

			return 0;
		};
		float64 toFloat64(){
			if (this->isInt){
				if (this->isSigned){
					switch (this->bytes){
						case 1:
							return this->pointer->int8;
						case 2:
							return this->pointer->int16;
						case 4:
							return this->pointer->int32;
						case 8:
							return this->pointer->int64;
					}
				}else{
					switch (this->bytes){
						case 1:
							return this->pointer->uint8;
						case 2:
							return this->pointer->uint16;
						case 4:
							return this->pointer->uint32;
						case 8:
							return this->pointer->uint64;
					}
				}
			}else{
				if (this->bytes == 8){
					return this->pointer->float64;
				}else{
					return this->pointer->float32;
				}
			}

			return 0;
		};

		bool IsTrue(){
			return this->pointer->uint64 == 1;
		};


		// Change the register's mode
		RegisterMode getMode(){
			if (this->isInt){
				if (this->isSigned){
					switch(this->bytes){
						case 1:
							return RegisterMode::int8;
						case 2:
							return RegisterMode::int16;
						case 4:
							return RegisterMode::int32;
						case 8:
							return RegisterMode::int64;
					}
				}else{
					switch(this->bytes){
						case 1:
							return RegisterMode::uint8;
						case 2:
							return RegisterMode::uint16;
						case 4:
							return RegisterMode::uint32;
						case 8:
							return RegisterMode::uint64;
					}
				}
			}else{
				if (this->bytes == 8){
					return RegisterMode::float64;
				}else{
					return RegisterMode::float32;
				}
			}

			return RegisterMode::invalid;
		};
		void setMode(RegisterMode mode){
			switch (mode){
				case RegisterMode::invalid:
					this->bytes    = 0;
					this->isInt    = true;
					this->isSigned = false;
				case RegisterMode::uint8:
					this->bytes    = 1;
					this->isInt    = true;
					this->isSigned = false;
					return;
				case RegisterMode::int8:
					this->bytes    = 1;
					this->isInt    = true;
					this->isSigned = true;
					return;
				case RegisterMode::uint16:
					this->bytes    = 2;
					this->isInt    = true;
					this->isSigned = false;
					return;
				case RegisterMode::int16:
					this->bytes    = 2;
					this->isInt    = true;
					this->isSigned = true;
					return;
				case RegisterMode::uint32:
					this->bytes    = 4;
					this->isInt    = true;
					this->isSigned = false;
					return;
				case RegisterMode::int32:
					this->bytes    = 4;
					this->isInt    = true;
					this->isSigned = true;
					return;
				case RegisterMode::uint64:
					this->bytes    = 8;
					this->isInt    = true;
					this->isSigned = false;
					return;
				case RegisterMode::int64:
					this->bytes    = 8;
					this->isInt    = true;
					this->isSigned = true;
					return;

				case RegisterMode::float32:
					this->bytes    = 4;
					this->isInt    = false;
					this->isSigned = true;
					return;
				case RegisterMode::float64:
					this->bytes    = 8;
					this->isInt    = false;
					this->isSigned = true;
					return;
			}
		};
};




// Get the register ID using a given input
int8 GetRegisterID(std::string str){
	if (str.size() != 2){
		return -1;
	}

	if (str[0] != 'r'){
		return -1;
	}

	#if RegisterCount != 9
		#error Internal Error: GetRegisterID is not meeting the register count
	#endif

	switch (str[1]){
		case 'A':
			return 0;
		case 'B':
			return 1;
		case 'C':
			return 2;
		case 'D':
			return 3;
		case 'E':
			return 4;
		case 'F':
			return 5;
		case 'G':
			return 6;
		case 'H':
			return 7;
		case 'I':
			return 8;
	}

	return -1;
};
