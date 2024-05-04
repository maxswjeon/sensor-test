#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#if __has_include(<format>)
#include <format>
using std::format;
#else
#include <fmt/format.h>
using fmt::format;
#endif

using std::cout;
using std::fstream;
using std::string;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		cout << "Usage: fw2header <firmware>";
		return 1;
	}

	fstream binary(argv[1], std::ios::in | std::ios::binary);
	if (!binary.is_open())
	{
		cout << "Failed to open firmware file";
		return 1;
	}

	string filepath = string(argv[1]);
	string filename = filepath.substr(filepath.find_last_of("/\\") + 1);

	fstream header(filename + ".h", std::ios::out);
	if (!header.is_open())
	{
		cout << "Failed to open ouput file";
		return 1;
	}

	string basename = filename.substr(0, filename.find_last_of("."));
	std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);

	cout << "basename: " << basename << std::endl;

	binary.seekg(0, std::ios::end);
	size_t length = binary.tellg();
	binary.seekg(0, std::ios::beg);

	header << "#ifndef __" << basename << "_H\n";
	header << "#define __" << basename << "_H\n\n";
	header << "#define FIRMWARE_LENGTH " << length << "\n\n";
	header << "const unsigned char firmware[] = {\n  ";
	for (size_t i = 0; i < length; ++i)
	{
		unsigned char byte;
		binary.read(reinterpret_cast<char *>(&byte), sizeof(byte));
		header << format("{:#04x}", byte) << ", ";
		if ((i + 1) % 16 == 0)
		{
			header << "\n  ";
		}
	}
	header << "\n};";
	header << "\n\n#endif";

	binary.close();
	header.close();

	return 0;
}