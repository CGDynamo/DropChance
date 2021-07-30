#include <iostream>
#include <time.h>
#include <string>
#include <thread>
#include <windows.system.threading.h>

bool ParseInputs(uint64_t argc, char* argv[], float_t* x, uint64_t* y, uint64_t* z)
{
	if (argc < 4 || argc >= 5)
		return false;

	*x = std::stof(argv[1]);
	*y = std::stoi(argv[2]);
	*z = std::stoi(argv[3]);

	//check for sanitation
	if (*x > 100 || *x < 0)
		return false;

	return true;
}

void PrintManPage()
{
	std::cerr
		<< "\n    itmchnce.exe [float - percentage] [integer - number of chests] [intiger - number of trials]\n"
		<< "\tA small program that checks multiple chests for item chances. The test checks only if item was found, not of what quantity.\n" 
		<< "\tIt performs this check in trials to catch anomolous outcomes. Nondeterministic, uses system time as seed.\n";
}

void RunTest(uint64_t* outcome, const float_t& percent, const int32_t& chests, const int32_t& trials)
{
	bool got = false;
	for (size_t i = 0; i < trials; i++)
	{
		for (size_t j = 0; j < chests; j++)
			if (got = ((rand() % 100) <= percent))break;

		outcome += (got);

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
	const int coreCount = std::thread::hardware_concurrency();

	std::cout << "\n" << percent << "\% drop over " << chests << " chest(s), " << trials << " times.\n";
	srand(time(NULL));

	


	float_t avg = (result / (float_t)trials) * 100.0f;

	std::cout << "\n Average chance: " << avg << "\%\n";

}
