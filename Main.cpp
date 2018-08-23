#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char ** argv) {
	std::string file_loc;
	__int64 entity_count;
	if (argc > 2) {
		std::stringstream parser;
		parser << argv[1];
		parser >> entity_count;
		file_loc.assign(argv[2]);

		std::ofstream result_file(file_loc, std::ios::binary);

		double bread;
		for (__int64 i = 0; i < entity_count; i++) {
			bread = (double)i;
			result_file.write((char *)&bread, 8);
		}

		result_file.close();
	}
	else {
		std::cerr << "Bad params" << std::endl;
	}
	return 0;
}