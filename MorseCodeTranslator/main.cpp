#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

// Timer Macros
#define START_TIMER std::chrono::time_point<std::chrono::system_clock> timerStart = std::chrono::system_clock::now()
#define END_TIMER std::chrono::time_point<std::chrono::system_clock> timerEnd = std::chrono::system_clock::now()
#define ELAPSED_TIME std::chrono::duration_cast<std::chrono::nanoseconds>(timerEnd - timerStart).count()


// Morse Code Map
std::map<char, std::string> g_morseCodeMap = {
	{'A', ".-"},
	{'B', "-..."},
	{'C', "-.-."},
	{'D', "-.."},
	{'E', "."},
	{'F', "..-."},
	{'G', "--."},
	{'H', "...."},
	{'I', ".."},
	{'J', ".---"},
	{'K', "-.-"},
	{'L', ".-.."},
	{'M', "--"},
	{'N', "-."},
	{'O', "---"},
	{'P', ".--."},
	{'Q', "--.-"},
	{'R', ".-."},
	{'S', "..."},
	{'T', "-"},
	{'U', "..-"},
	{'V', "...-"},
	{'W', ".--"},
	{'X', "-..-"},
	{'Y', "-.--"},
	{'Z', "--.."},
	{'1', ".----"},
	{'2', "..---"},
	{'3', "...--"},
	{'4', "....-"},
	{'5', "....."},
	{'6', "-...."},
	{'7', "--..."},
	{'8', "---.."},
	{'9', "----."},
	{'0', "-----"},
	{'.', ".-.-.-"},
	{',', "--..--"},
	{'?', "..--.."},
	{'!', "-.-.--"},
	{'\'', ".----."},
	{'"', ".-..-."},
	{'-', "-....-"},
	{'/', "-..-."},
	{'(', "-.--."},
	{')', "-.--.-"},
	{'&', ".-..."},
	{':', "---..."},
	{';', "-.-.-."},
	{'=', "-...-"},
	{'+', ".-.-."},
	{'_', "..--.-"},
	{'$', "...-..-"},
	{'@', ".--.-."},
	{' ', "  "}
};

std::string loadStringFromFile(std::string path)
{
	std::string line;
	std::string output;
	std::ifstream file(path);
	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			output += line + "\n";
		}
		file.close();
	}
	else
	{
		std::cerr << "Error opening file" << std::endl;
	}
	return output;
}

void translateToMorseCode(std::mutex& mtx, int chunkID, std::string input, std::vector<std::string>& output)
{
	std::string localOutput;
	for (char c : input)
	{
		localOutput += g_morseCodeMap[toupper(c)] + " ";
	}

	std::lock_guard<std::mutex> lock(mtx);
	output[chunkID] = localOutput;
}

int main(int argc, char* argv[])
{
	// Check for correct number of arguments
	if (argc < 2)
	{
		std::cerr << "Usage: MorseCodeTranslator.exe <path>" << std::endl;
		return 1;
	}

	std::string path = argv[1];

	// Check that file is a text file
	if (path.substr(path.find_last_of(".")) != ".txt")
	{
		std::cerr << "Error: File is not a text file" << std::endl;
		return 1;
	}

	std::cout << "Translating file to Morse Code..." << std::endl;

	// Load file
	std::string input = loadStringFromFile(path);

	// If input is empty, exit
	if (input.empty())
	{
		std::cerr << "Error: File is empty" << std::endl;
		return 1;
	}

	// Get number of processor cores
	unsigned int numCores = std::thread::hardware_concurrency();
	// Use two cores if available
	numCores = fmin(numCores, 2);

	// If input length is less than number of cores, set number of cores to input length
	if (input.length() < numCores)
	{
		numCores = input.length();
	}

	// Split input into chunks
	std::vector<std::string> inputChunks;
	unsigned int chunkSize = input.size() / numCores;
	unsigned int chunkStart = 0;
	unsigned int chunkEnd = chunkSize;
	for (unsigned int i = 0; i < numCores; i++)
	{
		inputChunks.push_back(input.substr(chunkStart, chunkEnd));
		chunkStart += chunkSize;
		chunkEnd += chunkSize;
	}

	// Create output strings
	std::vector<std::string> outputChunks(numCores);
	for (unsigned int i = 0; i < numCores; i++)
	{
		outputChunks[i] = "";
	}

	// Create threads
	std::mutex mtx;
	std::vector<std::thread> threads;
	START_TIMER;
	for (unsigned int i = 0; i < numCores; i++)
	{
		threads.push_back(std::thread(translateToMorseCode, std::ref(mtx), i, inputChunks[i], std::ref(outputChunks)));
	}

	// Join threads
	for (unsigned int i = 0; i < numCores; i++)
	{
		threads[i].join();
	}

	// Combine output
	std::string output;
	for (unsigned int i = 0; i < numCores; i++)
	{
		output += outputChunks[i];
	}

	END_TIMER;
	//std::cout << output << std::endl;
	std::cout << "Elapsed time: " << ELAPSED_TIME / 1000000 << "ms" << std::endl;

	return 0;
}