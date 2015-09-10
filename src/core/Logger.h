#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>

class Logger
{
public:

	template<typename T>
	Logger& operator << (T&& x) {
		if (m_LogToFile)
			m_LogFile << x;
		if (m_LogToConsole)
			std::cout << x;
		return *this;
	};

	Logger()
	{
		m_LogToFile = true;
		m_LogToConsole = true;

		m_LogFile = std::ofstream(get_date() + ".txt");

		if (!m_LogFile.is_open())
		{
			std::cout << "failed to open logg file.";
			m_LogToFile = false;
		}
	}

	~Logger()
	{
		m_LogFile.close();
	}

	void print(std::streambuf* text)
	{
		if (m_LogToFile)
			m_LogFile << text;
		if (m_LogToConsole)
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
	std::ofstream	m_LogFile;

	bool			m_LogToFile;
	bool			m_LogToConsole;
};