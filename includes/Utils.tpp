#ifndef UTILS_TPP
# define UTILS_TPP

template <typename T>
std::string toString(T value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
}

#endif