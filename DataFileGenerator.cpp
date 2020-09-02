// DataFileGenerator.cpp
//

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


const std::string g_sUsageMessage = "\t<# of elements> <output file path>\n\t\tor\n\
\t<# of elements> <output file path> <min> <max>\n\t\tor\n\
\t<# of elements> <output file path> <min> <max> <data format>\n\t\tor\n\
\t<# of elements> <output file path> <min> <max> <data format> <seed>\n\n\
Supported data formats are:\n\
\tbyte, char, character -> signed 8b integer\n\
\tdouble                -> 64b floating point\n\
\tfloat                 -> 32b floating point\n\
\tint, integer          -> signed 32b integer\n\
\tlong                  -> signed 64b integer\n\n\
Defaults:\n\
\tDistribution: [100, 1000)\n\
\tData format: double\n";


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
	long long seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
		std::stringstream paramParser;
		paramParser << argv[2];
		paramParser >> retval.entity_count;
		retval.file_loc = std::make_unique<std::string>(argv[1]);

		break;
	}
	}

	return retval;
}


auto instantiate_generator(Params const& userParams) {
	using namespace std;

	mt19937_64 oRNG(userParams.seed);
	uniform_real_distribution oDist(
		userParams.uniform_dist_min_inclusive,
		userParams.uniform_dist_max_exclusive);
	return bind(oDist, oRNG);
}


void display_usage_message(const char program_name[]) {
	using namespace std;

	cout << "Generates a binary data file with \n\nUsage:\n"
		<< program_name << " ...\n" << g_sUsageMessage << endl;
}


int main(int argc, const char* const argv[]) {
	using namespace std;

	auto userParams = parse_command_line_args(argc, argv);

	if (userParams.bad_params) {
		std::cerr << "Bad params" << std::endl;
		display_usage_message(argv[0]);
		return -1;
	}

	auto generate = instantiate_generator(userParams);
	ofstream result_file(*userParams.file_loc, ios::binary);

	if (!result_file.is_open()) {
		std::cerr << "Could not open file \"" << *userParams.file_loc << "\"" << std::endl;
		return -2;
	}

	size_t writeCount = 0;

	double randReal;
	for (long long i = 0; i < userParams.entity_count; ++i) {
		randReal = generate();
		switch (userParams.number_format) {
		case OutputFormat::USE_CHAR: {
			result_file.write((char*)&randReal, sizeof(char));
			break;
		}
		case OutputFormat::USE_FLOAT: {
			result_file.write((char*)&randReal, sizeof(float));
			break;
		}
		case OutputFormat::USE_INT: {
			result_file.write((char*)&randReal, sizeof(int));
			break;
		}
		case OutputFormat::USE_LONG: {
			result_file.write((char*)&randReal, sizeof(long long));
			break;
		}
		default: {
			result_file.write((char*)&randReal, sizeof(double));
			break;
		}
		}
		++writeCount;
	}

	result_file.close();

	cout << "File \"" << *userParams.file_loc << "\" written\n\twith " << writeCount << " elements." << endl;

	return 0;
}
