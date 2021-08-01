#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <random>


/// <summary>
/// Interpret passed values
/// </summary>
/// <param name="argc"> Number of passed input strings </param>
/// <param name="argv"> Collection of passed input strings </param>
/// <param name="chance"> The drop percentage </param>
/// <param name="chests"> The amount of looted containers to pull from </param>
/// <param name="trials"> The iterations to test over</param>
/// <returns></returns>
bool ParseInputs(uint64_t argc, char* argv[], float_t* chance, uint64_t* chests, uint64_t* trials)
{
	if (argc < 4 || argc >= 5)
		return false;

	*chance = std::stof(argv[1]);
	*chests = std::stoi(argv[2]);
	*trials = std::stoi(argv[3]);

	//check for sanitation of inputs
	if (*chance > 100 || *chance < 0 || *chests < 1 || *trials < 1)
		return false;

	//offer insight/guidance
	if (*trials < 3621)
		std::cout << "\n Low trial counts may lead to unexpected and abnormal results.\n Results may be more accurate with greater than " << *trials << " tests\n";

	return true;
}

/// <summary>
/// Output if fault occured with inputs
/// </summary>
void PrintManPage()
{
	std::cerr
		<< "\n\tdropchance.exe [float - percentage] [integer - number of chests] [intiger - number of trials]\n"
		<< "  Virtually checks multiple chests for item chances. The test checks only if item was found, not of what quantity.\n"
		<< "  It performs this check in trials to catch anomolous outcomes. Nondeterministic, uses system time as seed.\n";
}

/// <summary>
/// The data block handed off to each thread
/// </summary>
struct TestData {
	uint64_t* outcome;
	float_t percent;
	uint64_t chests;
	uint64_t trials;
};

/// <summary>
/// Threadoperation
/// </summary>
/// <param name="data"></param>
void RunTest(TestData data)
{
	//got allows for early termination of operation
	bool got = false;

	//get and assign random value for each thread
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> randVal(0, 1000000);
	
	//perform trials alotted to thread
	for (uint64_t i = 0; i < data.trials; i++)
	{
		for (uint64_t j = 0; j < data.chests; j++)
			if (got = ((randVal(gen) *0.0001f) <= data.percent))break;
		*data.outcome += (got);
	}
}


int main(int32_t argc, char* argv[])
{
	//variable initialization and early assignment;
	float_t percent = 10.0f;
	uint64_t chests = 10,
		trials = 10,
		result = 0;

	//interpret inputs, return blab about correct useage
	if (!ParseInputs(argc, argv, &percent, &chests, &trials))
	{
		//print man
		PrintManPage();
		return -1;
	}

	//set up if inputs are valid
	std::cout << "\n " << percent << "% drop over " << chests << " chest(s), " << trials << " times.\n";

	//get start time for operation(s)
	auto startOp = std::chrono::high_resolution_clock::now();

	//get native hardware count
	const uint64_t coreCount = std::thread::hardware_concurrency();

	//create and assign collections for threaded ops
	std::thread* threads = new std::thread[coreCount + 1];
	TestData* data = new TestData[coreCount + 1];
	uint64_t* results = new uint64_t[coreCount + 1]{0};

	//calc the trials needed from each core
	uint64_t split = trials / coreCount;

	//generate and execute threads, + 1 allows for basic operation such that hardware_concurrency() returns 0
	//as well, it allows for a remainder operation so that the trials aren't reduced to a multiple of available cores
	for (uint64_t i = 0; i < coreCount + 1; i++)
	{
		//if last entry *and* more than one core, throw remainder at thread
		data[i] = { &results[i], percent, chests, (i < coreCount && coreCount > 1) ? split : trials % coreCount };
		threads[i] = std::thread(RunTest, data[i]);
	}

	//loop through the created threads
	for (uint64_t i = 0; i < coreCount + 1; i++)
	{
		//check if they are done
		threads[i].join();
		//sum calculation results
		result += results[i];
	}

	//calc the percentage of success along trials
	float_t avg = (result / (float_t)trials) * 100.0f;

	//get end of operation time and get the difference
	auto stopOp = std::chrono::high_resolution_clock::now();
	uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stopOp - startOp).count();

	//write out the chance and time in an easy to understand format
	std::cout << "\n Average drop chance: " << avg << "%";
	if (elapsed > 10000000)
		std::cout << " found in " << elapsed / 1000000 << " seconds\n";
	else if (elapsed > 10000)
		std::cout << " found in " << elapsed / 1000 << " milliseconds\n";
	else
		std::cout << " found in " << elapsed << " microseconds\n";

	//rid ourselves of those colelctions now that we're through
	delete[] threads;
	delete[] results;
	delete[] data;
}
