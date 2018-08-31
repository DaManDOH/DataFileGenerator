// main.cpp

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

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
	long long int seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

Params parse_params(int argc, char ** argv) {
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

			if (argc > 6) {
				argc = 6;
			}

			std::stringstream paramParser;
			for (int i = argc - 1; i >= 1; i--) {
				paramParser << argv[i] << " ";
			}

			switch (argc) {
			case 6:
				paramParser >> retval.seed;
			case 5:
				paramParser >> retval.uniform_dist_max_exclusive;
				paramParser >> retval.uniform_dist_min_inclusive;
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

	auto userParams = parse_params(argc, argv);

	if (userParams.good_params) {
		std::cout << "Good params" << std::endl;

		std::mt19937_64 oRNG(userParams.seed);
		std::uniform_real_distribution<> oDist(userParams.uniform_dist_min_inclusive, userParams.uniform_dist_max_exclusive);
		auto generate = std::bind(oDist, oRNG);

		std::ofstream result_file(userParams.file_loc, std::ios::binary);

		long long int writeCount = 0ll;
		double randReal;
		for (long long int i = 0; i < userParams.entity_count; i++) {
			randReal = generate();
			result_file.write((char *)&randReal, 8);
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