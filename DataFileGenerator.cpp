// DataFileGenerator.cpp

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>

enum OutputFormat {
	USE_DOUBLE, // default
	USE_CHAR,
	USE_FLOAT,
	USE_INT,
	USE_LONG
};

std::map<std::string, OutputFormat> g_oStringToOutputFormatMap = {
	{"", USE_DOUBLE},
	{"byte", USE_CHAR},
	{"char", USE_CHAR},
	{"character", USE_CHAR},
	{"double", USE_DOUBLE},
	{"float", USE_FLOAT},
	{"int", USE_INT},
	{"integer", USE_INT},
	{"long", USE_LONG}
};

struct Params {
	// Good params flag
	bool good_params;

	// First param
	long long int entity_count;

	// Second param
	std::string file_loc;

	// Third param (optional)
	double uniform_dist_min_inclusive = 100.0;

	// Fourth param (optional)
	double uniform_dist_max_exclusive = 1000.0;

	// Fifth param (optional)
	OutputFormat number_format = USE_DOUBLE;

	// Sixth param (optional)
	long long int seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

std::istream & operator>>(std::istream & in, OutputFormat & rhs) {
	std::string outputFormatString;
	in >> outputFormatString;
	std::transform(outputFormatString.begin(), outputFormatString.end(), outputFormatString.begin(), ::tolower);
	OutputFormat check = g_oStringToOutputFormatMap[outputFormatString];
	rhs = check;
	return in;
}

Params parse_command_line_args(int argc, char ** argv) {
	Params retval;

	if (argc < 3) {
		retval.good_params = false;
	}
	else {

		/*
		Must specify both the min and the max.
		*/
		if (argc == 4) {
			retval.good_params = false;
		}
		else {
			retval.good_params = true;

			if (argc > 7) {
				argc = 7;
			}

			std::stringstream paramParser;
			for (int i = argc - 1; i >= 1; i--) {
				paramParser << argv[i] << " ";
			}

			switch (argc) {
			case 7:
				paramParser >> retval.seed;
                [[fallthrough]];
			case 6:
				paramParser >> retval.number_format;
                [[fallthrough]];
			case 5:
				paramParser >> retval.uniform_dist_max_exclusive;
				paramParser >> retval.uniform_dist_min_inclusive;
                [[fallthrough]];
			case 3:
				paramParser >> retval.file_loc;
				paramParser >> retval.entity_count;
				break;
			}
		}
	}

	return retval;
}

int main(int argc, char ** argv) {
	int exitCode = 0;

	auto userParams = parse_command_line_args(argc, argv);

	if (userParams.good_params) {
		std::cout << "Good params" << std::endl;

		std::mt19937_64 oRNG(userParams.seed);
		std::uniform_real_distribution<> oDist(userParams.uniform_dist_min_inclusive, userParams.uniform_dist_max_exclusive);
		auto generate = std::bind(oDist, oRNG);

		std::ofstream result_file(userParams.file_loc, std::ios::binary);

		size_t writeCount = (size_t)0;
		double randReal;
		for (long long int i = 0; i < userParams.entity_count; i++) {
			randReal = generate();
			switch (userParams.number_format) {
			case USE_CHAR: {
				char cast = (char)randReal;
				result_file.write((char *)&cast, sizeof(char));
				break;
			}
			case USE_FLOAT: {
				float cast = (float)randReal;
				result_file.write((char *)&cast, sizeof(float));
				break;
			}
			case USE_INT: {
				int cast = (int)randReal;
				result_file.write((char *)&cast, sizeof(int));
				break;
			}
			default: {
				result_file.write((char *)&randReal, sizeof(double));
				break;
			}
			}
			writeCount++;
		}

		result_file.close();

		std::cout << "File " << userParams.file_loc << " (" << writeCount << " elements) written." << std::endl;
	}
	else {
		std::cerr << "Bad params" << std::endl;
		exitCode = -1;
	}

	return exitCode;
}
