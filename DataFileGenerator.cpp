// DataFileGenerator.cpp
//

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
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
	long long entity_count;

	// Second param
	std::string file_loc;

	// Third param (optional)
	double uniform_dist_min_inclusive = 100.0;

	// Fourth param (optional)
	double uniform_dist_max_exclusive = 1000.0;

	// Fifth param (optional)
	OutputFormat number_format = USE_DOUBLE;

	// Sixth param (optional)
	long long seed = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
};


std::istream & operator>>(std::istream & in, OutputFormat & rhs) {
	std::string outputFormatString;
	in >> outputFormatString;
	std::transform(outputFormatString.begin(), outputFormatString.end(), outputFormatString.begin(), ::tolower);
	OutputFormat check = g_oStringToOutputFormatMap[outputFormatString];
	rhs = check;
	return in;
}


Params parse_command_line_args(int argc, char* argv[]) {
	Params retval;

	if (argc < 3) {
		retval.good_params = false;
	}
	else {

		/*
		When distribution min is specified, distribution max must also be specified.
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
			for (auto i = argc - 1; i >= 1; --i) {
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


auto instantiate_generator(const Params &userParams) {
	using namespace std;

	mt19937_64 oRNG(userParams.seed);
	uniform_real_distribution<> oDist(
		userParams.uniform_dist_min_inclusive,
		userParams.uniform_dist_max_exclusive);
	return bind(oDist, oRNG);
}


void display_usage_message(const char program_name[]) {
	using namespace std;

	cout << "Generates a binary data file with \n\nUsage:\n"
		<< program_name << " ...\n" << g_sUsageMessage << endl;
}


int main(int argc, char* argv[]) {
	using namespace std;

	int exitCode = 0;

	auto userParams = parse_command_line_args(argc, argv);

	if (userParams.good_params) {
		auto generate = instantiate_generator(userParams);
		ofstream result_file(userParams.file_loc, ios::binary);
		size_t writeCount = 0;

		double randReal;
		for (long long i = 0; i < userParams.entity_count; ++i) {
			randReal = generate();
			switch (userParams.number_format) {
			case USE_CHAR: {
				result_file.write((char *)&randReal, sizeof(char));
				break;
			}
			case USE_FLOAT: {
				result_file.write((char *)&randReal, sizeof(float));
				break;
			}
			case USE_INT: {
				result_file.write((char *)&randReal, sizeof(int));
				break;
			}
			case USE_LONG: {
				result_file.write((char *)&randReal, sizeof(long long));
				break;
			}
			default: {
				result_file.write((char *)&randReal, sizeof(double));
				break;
			}
			}
			++writeCount;
		}

		result_file.close();

		cout << "File " << userParams.file_loc << " written\n\twith " << writeCount << " elements." << endl;
	}
	else {
		cerr << "Bad params" << endl;
		display_usage_message(argv[0]);
		exitCode = -1;
	}

	return exitCode;
}
