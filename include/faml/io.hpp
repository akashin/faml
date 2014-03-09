#ifndef IO_HPP
#define IO_HPP

#include <fstream>
#include <sstream>

#include "data/table.hpp"

namespace faml {

std::vector<std::string> readCSVHeader(std::ifstream &inputStream) {
	std::string line;
	std::getline(inputStream, line);

	std::vector<std::string> header;

	std::istringstream ss(line);
	std::string token;
	while (std::getline(ss, token, ',')) {
		header.push_back(token);
	}

	return header;
}

Table< std::vector<std::string> > readCSV(const std::string &filename) {
	typedef std::vector<std::string> RowType;
	std::ifstream inputStream(filename);
	Table<RowType> table(readCSVHeader(inputStream));
	size_t headerLength = table.columnsNumber();

	while (!inputStream.eof()) {
		std::string line;
		getline(inputStream, line);

		if (inputStream.eof()) {
			break;
		}

		std::istringstream ss(line);

		RowType data(headerLength);

		for (int i = 0; i < headerLength; ++i) {
			std::getline(ss, data[i], ',');
		}

		table.addRow(data);
	}
	return table;
}

} // namespace faml

#endif // IO_HPP
