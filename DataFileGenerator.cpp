// DataFileGenerator.cpp

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>

enum class OutputFormat {
	USE_DOUBLE, // default
	USE_CHAR,
	USE_FLOAT,
	USE_INT,
	USE_LONG
};

std::map<std::string, OutputFormat> g_oStringToOutputFormatMap = {
	{"", OutputFormat::USE_DOUBLE},
	{"byte", OutputFormat::USE_CHAR},
	{"char", OutputFormat::USE_CHAR},
	{"character", OutputFormat::USE_CHAR},
	{"double", OutputFormat::USE_DOUBLE},
	{"float", OutputFormat::USE_FLOAT},
	{"int", OutputFormat::USE_INT},
	{"integer", OutputFormat::USE_INT},
	{"long", OutputFormat::USE_LONG}
};

struct Params {
	// Parameter sentinel
	bool bad_params = true;

	// First param
	long long int entity_count = -1;

	// Second param
	std::unique_ptr<std::string> file_loc;

	// Third param (optional)
	double uniform_dist_min_inclusive = 100.0;

	// Fourth param (optional)
	double uniform_dist_max_exclusive = 1000.0;

	// Fifth param (optional)
	OutputFormat number_format = OutputFormat::USE_DOUBLE;

	// Sixth param (optional)
	long long int seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

std::istream& operator>>(std::istream& in, OutputFormat& rhs) {
	std::string outputFormatString;
	in >> outputFormatString;
	std::transform(outputFormatString.begin(), outputFormatString.end(), outputFormatString.begin(), ::tolower);
	rhs = g_oStringToOutputFormatMap[outputFormatString];
	return in;
}

Params parse_command_line_args(int argc, const char* const argv[]) {
	Params retval;

	/*
	 * Must specify # of elements and destination file.
	 */
	if (argc < 3) {
		retval.bad_params = true;
		return retval;
	}

	/*
	 * When lower bound of range is specified, both min and max are required.
	 */
	if (argc == 4) {
		retval.bad_params = true;
		return retval;
	}

	retval.bad_params = false;

	if (argc > 7) {
		argc = 7;
	}

	switch (argc) {
	case 7: {
		std::stringstream paramParser;
		paramParser << argv[6];
		paramParser >> retval.seed;
	}
		[[fallthrough]];
	case 6: {
		std::stringstream paramParser;
		paramParser << argv[5];
		paramParser >> retval.number_format;
	}
		[[fallthrough]];
	case 5: {
		std::stringstream paramParser;
		paramParser << argv[4] << " " << argv[3];
		paramParser >> retval.uniform_dist_max_exclusive;
		paramParser >> retval.uniform_dist_min_inclusive;
	}
		[[fallthrough]];
	case 3: {
		retval.file_loc = std::make_unique<std::string>(argv[2]);
		std::stringstream paramParser;
		paramParser << argv[1];
		paramParser >> retval.entity_count;

		break;
	}
	}

	return retval;
}

int main(int argc, const char* const argv[]) {

	auto userParams = parse_command_line_args(argc, argv);

	if (userParams.bad_params) {
		std::cerr << "Bad params" << std::endl;
		return -1;
	}

	std::cout << "Good params" << std::endl;

	std::mt19937_64 oRNG(userParams.seed);
	std::uniform_real_distribution oDist(userParams.uniform_dist_min_inclusive, userParams.uniform_dist_max_exclusive);
	auto generate = std::bind(oDist, oRNG);

	std::ofstream result_file(*userParams.file_loc, std::ios::binary);

	if (!result_file.is_open()) {
		std::cerr << "Could not open file \"" << *userParams.file_loc << "\"" << std::endl;
		return -2;
	}

	size_t writeCount = 0;
	double randReal;
	for (long long int i = 0; i < userParams.entity_count; i++) {
		randReal = generate();
		switch (userParams.number_format) {
		case OutputFormat::USE_CHAR: {
			char cast = (char)randReal;
			result_file.write((char*)&cast, sizeof(char));
			break;
		}
		case OutputFormat::USE_FLOAT: {
			float cast = (float)randReal;
			result_file.write((char*)&cast, sizeof(float));
			break;
		}
		case OutputFormat::USE_INT: {
			int cast = (int)randReal;
			result_file.write((char*)&cast, sizeof(int));
			break;
		}
		default: {
			result_file.write((char*)&randReal, sizeof(double));
			break;
		}
		}
		writeCount++;
	}

	result_file.close();

	std::cout << "File \"" << *userParams.file_loc << "\" (" << writeCount << " elements) written." << std::endl;

	return 0;
}
