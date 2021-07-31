#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <random>

/// <summary>
/// Parses inputs passed on command line
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <returns> Parsed Correctly </returns>
bool ParseInputs(uint64_t argc, char* argv[], float_t* x, uint64_t* y, uint64_t* z)
{
	if (argc < 4 || argc >= 5)
		return false;

	*x = std::stof(argv[1]);
	*y = std::stoi(argv[2]);
	*z = std::stoi(argv[3]);

	//check for sanitation of inputs
	if (*x > 100 || *x < 0 || *y < 1 || *z < 1)
		return false;

	//offer quidance
	if (*z < 1337)
		std::cout << "\n Low trial counts may lead to unexpected and abnormal results.\n Results may be more accurate with greater than " << *z << " tests\n";

	return true;
}

/// <summary>
/// Output if fault occured with entry
/// </summary>
void PrintManPage()
{
	std::cerr
		<< "\n\tdropchance.exe [float - percentage] [integer - number of chests] [intiger - number of trials]\n"
		<< "  Virtually checks multiple chests for item chances. The test checks only if item was found, not of what quantity.\n"
		<< "  It performs this check in trials to catch anomolous outcomes. Nondeterministic, uses system time as seed.\n";
}

struct TestData {
	uint64_t* outcome;
	float_t percent;
	uint64_t chests;
	uint64_t trials;
};

void RunTest(TestData data)
{
	bool got = false;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> randVal(0, 10000);
	for (uint64_t i = 0; i < data.trials; i++)
	{
		for (uint64_t j = 0; j < data.chests; j++)
			if (got = ((randVal(gen) *0.01f) <= data.percent))break;

		*data.outcome += (got);
	}
}


int main(int32_t argc, char* argv[])
{
	float_t percent = 10.0f;
	uint64_t chests = 10,
		trials = 10,
		result = 0;

	if (!ParseInputs(argc, argv, &percent, &chests, &trials))
	{
		//print man
		PrintManPage();
		return -1;
	}

	//set up if inputs are valid
	std::cout << "\n " << percent << "% drop over " << chests << " chest(s), " << trials << " times.\n";

	auto startOp = std::chrono::high_resolution_clock::now();

	const uint64_t coreCount = std::thread::hardware_concurrency();

	std::thread* threads = new std::thread[coreCount + 1];
	TestData* data = new TestData[coreCount + 1];
	uint64_t* results = new uint64_t[coreCount + 1]{0};

	uint64_t split = trials / coreCount;

	for (uint64_t i = 0; i < coreCount + 1; i++)
	{
		data[i] = { &results[i], percent, chests, (i < coreCount - 1 && coreCount > 1) ? split : trials % coreCount };//if last entry and more than one core, throw remainder
		threads[i] = std::thread(RunTest, data[i]);
	}

	for (uint64_t i = 0; i < coreCount + 1; i++)
	{
		threads[i].join();
		result += results[i];
	}

	auto stopOp = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stopOp - startOp).count();

	float_t avg = (result / (float_t)trials) * 100.0f;

	std::cout << "\n Average drop chance: " << avg << "%";
	if (elapsed > 10000000)
		std::cout << " found in " << elapsed / 1000000 << " seconds\n";
	else if (elapsed > 10000)
		std::cout << " found in " << elapsed / 1000 << " milliseconds\n";
	else
		std::cout << " found in " << elapsed << " microseconds\n";

	delete[] threads;
	delete[] results;
	delete[] data;
}
