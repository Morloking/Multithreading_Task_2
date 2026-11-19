#include <chrono>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>


// If I’ve understood correctly, we are working specifically with vectors of the same length. //

namespace {
	constexpr const char* kWelcomeMsg = "Number of hardware cores - ";
	constexpr const char* kSizeErrorMsg = "Vectors have different sizes. Program execution is impossible";
	constexpr const char* kNumOfThreadsErrorMsg = "The number of threads cannot be zero";
	constexpr const char* kErrorPrefixMsg = "Error: ";

	constexpr const char* kOneThousandMsg = "1'000";
	constexpr const char* kTenThousandMsg = "10'000";
	constexpr const char* kOneHundredThousandMsg = "100'000";
	constexpr const char* kOneMillionMsg = "1'000'000";

	constexpr const char* kOneThreadMsg = "1 thread ";
	constexpr const char* kTwoThreadsMsg = "2 threads ";
	constexpr const char* kFourThreadsMsg = "4 threads ";
	constexpr const char* kEightThreadsMsg = "8 threads ";
	constexpr const char* kSixteenThreadsMsg = "16 threads ";

	constexpr const char kTimePostfix = 's';

	constexpr const int kDefaultSpacerSize = 15;

	template <typename T>
	using VecPair = std::pair <std::vector<T>, std::vector<T>>;

	void PrintHeader() {
		std::cout << kWelcomeMsg << std::thread::hardware_concurrency() << "\n";
		std::cout << std::setw(kDefaultSpacerSize) << ""
			<< std::setw(kDefaultSpacerSize) << kOneThousandMsg
			<< std::setw(kDefaultSpacerSize) << kTenThousandMsg
			<< std::setw(kDefaultSpacerSize) << kOneHundredThousandMsg
			<< std::setw(kDefaultSpacerSize) << kOneMillionMsg << "\n";
	}
	std::pair<std::size_t, std::size_t> CalculateEndAndStartP(std::size_t num_threads, std::size_t curr_thread, std::size_t size_v, std::size_t block_size) {
		std::size_t start_point = curr_thread * block_size;
		std::size_t end_point = start_point + block_size;
		if (curr_thread == num_threads - 1) {
			end_point = size_v;
		}
		return std::pair<std::size_t, std::size_t>(start_point, end_point);
	}
	template <typename T>
	void SumRange(
		const std::vector<T>& vec1,
		const std::vector<T>& vec2,
		std::vector<T>& result,
		size_t start_point,
		size_t end_point) {
		for (std::size_t i = start_point; i < end_point; ++i) {
			result[i] = vec1[i] + vec2[i];
		}
	}
	template <typename T>
	std::vector<T> UniversalSum(const std::vector<T>& vec1, const std::vector<T>& vec2, std::size_t num_threads) {
		if (vec1.size() != vec2.size()) { throw std::runtime_error(kSizeErrorMsg); }
		if (num_threads == 0) { throw std::runtime_error(kNumOfThreadsErrorMsg); }

		std::size_t size = vec1.size();
		std::vector<T> result(size);

		if (num_threads == 1) { // Default case = 1 thread, no parallelizm
			SumRange(vec1, vec2, result, 0, size);
			return result;
		}
		
		std::size_t block_size = size / num_threads;
		std::vector<std::thread> threads;
		threads.reserve(num_threads);
		

		for (std::size_t i = 0; i < num_threads; ++i) {
			auto start_and_end = CalculateEndAndStartP(num_threads, i, size, block_size);
			threads.emplace_back(
				SumRange<T>,
				std::cref(vec1),
				std::cref(vec2),
				std::ref(result),
				start_and_end.first,
				start_and_end.second
			);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		return result;
	}
	template <typename Function>
	auto MeasureTime(Function sum_func) {
		auto start_time = std::chrono::high_resolution_clock::now();
		sum_func();
		auto end_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(end_time - start_time);
	}
	template <typename T>
	void PrintRow(const char* num_of_threads_str, std::size_t num_of_threads, const std::vector<VecPair<T>>& pairs) {
		std::cout << std::setw(kDefaultSpacerSize) << num_of_threads_str << std::fixed << std::setprecision(7);
		for (const auto& [vec_1, vec_2] : pairs) {
			auto time = MeasureTime([&] {
				std::vector<T> result = UniversalSum(vec_1, vec_2, num_of_threads);
				volatile T sink = result.empty() ? T{} : result.back(); // Protection against compiler optimization—since, as I understand from the task, 
				(void)sink;                                             // the outputs are not actually used.
				});
			std::cout << std::setw(kDefaultSpacerSize) << time.count() << kTimePostfix;
		}
		std::cout << "\n";
	}
}

int main() {
	try {
		std::vector<VecPair<int>> pairs = {
			{ std::vector<int>(1000, 1), std::vector<int>(1000, 1) },
			{ std::vector<int>(10'000, 1), std::vector<int>(10'000, 1) },
			{ std::vector<int>(100'000, 1), std::vector<int>(100'000, 1) },
			{ std::vector<int>(1'000'000, 1), std::vector<int>(1'000'000, 1) }
		};
		PrintHeader();
		PrintRow(kOneThreadMsg, 1, pairs);
		PrintRow(kTwoThreadsMsg, 2, pairs);
		PrintRow(kFourThreadsMsg, 4, pairs);
		PrintRow(kEightThreadsMsg, 8, pairs);
		PrintRow(kSixteenThreadsMsg, 16, pairs);
	}
	catch (const std::exception& e) {
		std::cerr << kErrorPrefixMsg << e.what() << "\n";
		return 1;
	}
	return 0;
}
