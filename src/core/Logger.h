#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <stdexcept>

class Logger
{
public:

	template<typename T>
	Logger& operator << (T&& x) {
		if (loggToFile)
			loggFile << x;
		if (loggToConsole)
			std::cout << x;
		return *this;
	};

	Logger()
	{
		loggToFile = true;
		loggToConsole = true;

		loggFile = std::ofstream(get_date() + ".txt");

		if (!loggFile.is_open())
		{
			std::cout << "failed to open logg file.";
			loggToFile = false;
		}
	}

	~Logger()
	{
		loggFile.close();
	}

	void print(std::streambuf* text)
	{
		if (loggToFile)
			loggFile << text;
		if (loggToConsole)
			std::cout << text;
	}

	std::string get_date()
	{
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		char date_time[30];
		strftime(date_time, sizeof(date_time), "%y-%m-%d %H.%M.%S", t);

		return std::string(date_time);
	}

private:
	std::ofstream loggFile;

	bool loggToFile;
	bool loggToConsole;
};

#endif