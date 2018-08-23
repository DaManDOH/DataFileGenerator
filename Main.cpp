#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

int main(int argc, char ** argv) {
	long long nSeed;
	nSeed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//nSeed = 0x37373737;
	std::mt19937_64 oRNG(nSeed);
	std::uniform_real_distribution<> oDist(100, 999);
	auto generate = std::bind(oDist, oRNG);
	std::string file_loc;
	__int64 entity_count;
	if (argc > 2) {
		std::stringstream parser;
		parser << argv[1];
		parser >> entity_count;
		file_loc.assign(argv[2]);

		std::ofstream result_file(file_loc, std::ios::binary);

		__int64 writeCount = 0ll;
		double bread;
		char * breadPtr = (char *)&bread;
		char breadCharArr[9];
		breadCharArr[8] = 0;
		for (__int64 i = 0; i < entity_count; i++) {
			bread = generate();
			std::copy(breadPtr, breadPtr + 8, breadCharArr);
			result_file.write((char *)&bread, 8);
			writeCount++;
		}

		result_file.close();

		std::cout << "File " << file_loc << " (" << writeCount << " elements) written." << std::endl;
	}
	else {
		std::cerr << "Bad params" << std::endl;
	}

	char buff[2];
	std::cin.read(buff, 1);

	return 0;
}