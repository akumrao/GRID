

#ifndef RTC_IMPL_UTILS_H
#define RTC_IMPL_UTILS_H
#include <random>
#include <string>
#include <vector>
    using namespace std;

namespace rtc::utils {

std::vector<string> explode(const string &str, char delim);
string implode(const std::vector<string> &tokens, char delim);

// Decode URL percent-encoding (RFC 3986)
// See https://www.rfc-editor.org/rfc/rfc3986.html#section-2.1
string url_decode(const string &str);


// Return a random seed sequence
std::seed_seq random_seed();

template <typename Generator, typename Result = typename Generator::result_type>
struct random_engine_wrapper {
	Generator &engine;
	using result_type = Result;
	static constexpr result_type min() { return static_cast<Result>(Generator::min()); }
	static constexpr result_type max() { return static_cast<Result>(Generator::max()); }
	inline result_type operator()() { return static_cast<Result>(engine()); }
	inline void discard(unsigned long long z) { engine.discard(z); }
};

// Return a wrapped thread-local seeded random number generator
template <typename Generator = std::mt19937, typename Result = typename Generator::result_type>
auto random_engine() {
	static thread_local std::seed_seq seed = random_seed();
	static thread_local Generator engine{seed};
	return random_engine_wrapper<Generator, Result>{engine};
}


} // namespace rtc::utils

#endif
