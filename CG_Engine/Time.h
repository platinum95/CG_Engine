#pragma once
#include <chrono>

namespace GL_Engine {
	template<typename TimeResolution>
	class Stopwatch {
		using TimeStamp = std::chrono::time_point <std::chrono::system_clock>;
	public:
		Stopwatch() {
			LastTime = std::chrono::system_clock::now();
		}
		void Initialise() {
			LastTime = std::chrono::system_clock::now();
		}
		TimeResolution MeasureTime() {
			auto diff = std::chrono::duration_cast<TimeResolution>(std::chrono::system_clock::now() - LastTime);
			LastTime = std::chrono::system_clock::now();
			return diff;
		}
	private:
		TimeStamp LastTime;
	};

}

