#pragma once

#include <iostream>
#include <chrono>
#include <string>

class timer
{
	typedef std::chrono::steady_clock::time_point   tp;
	typedef std::chrono::duration<double>           dd;
	typedef std::chrono::steady_clock               sc;

private:
	tp _begin = sc::now();
	dd _span = dd(0);

public:
	void start()
	{
		_begin = sc::now();
	}

	void pause()
	{
		tp _end = sc::now();
		_span += std::chrono::duration_cast<dd>(_end - _begin);
	}

	void stop(std::string head = std::string(), std::string tail = std::string())
	{
		tp _end = sc::now();
		_span += std::chrono::duration_cast<dd>(_end - _begin);
		std::cout << head << ": " << 1000 * _span.count() << " ms" << tail << std::endl;
		_span = dd(0);
	}

	void restart(std::string head = std::string(), std::string tail = std::string())
	{
		stop(head, tail);
		_begin = sc::now();
	}
};