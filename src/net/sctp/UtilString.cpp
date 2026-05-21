

#include "UtilString.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <thread>
#include <chrono>

namespace  rtc::utils  {



std::vector<string> explode(const string &str, char delim) {
	std::vector<std::string> result;
	std::istringstream ss(str);
	string token;
	while (std::getline(ss, token, delim))
		result.push_back(token);

	return result;
}

string implode(const std::vector<string> &tokens, char delim) {
	string sdelim(1, delim);
	std::ostringstream ss;
	std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<string>(ss, sdelim.c_str()));
	string result = ss.str();
	if (result.size() > 0)
		result.resize(result.size() - 1);

	return result;
}

std::seed_seq random_seed() {
	std::vector<unsigned int> seed;

	// Seed with random device
	try {
		// On some systems an exception might be thrown if the random_device can't be initialized
		std::random_device device;
		// 128 bits should be more than enough
		std::generate_n(std::back_inserter(seed), 4, std::ref(device));
	} catch (...) {
		// Ignore
	}

	// Seed with high-resolution clock
	//using std::chrono::high_resolution_clock;
	seed.push_back(
            static_cast<unsigned int>(std::chrono::high_resolution_clock::now()
                                          .time_since_epoch()
                                          .count()));

	// Seed with thread id
	seed.push_back(
	    static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id())));

	return std::seed_seq(seed.begin(), seed.end());
}

}
