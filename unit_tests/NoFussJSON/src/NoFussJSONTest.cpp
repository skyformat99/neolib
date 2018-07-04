#include <iostream>
#include <fstream>
#include <neolib/json.hpp>

template class neolib::basic_quick_string<char>;

int main(int argc, char** argv)
{
	try
	{
		const std::string tests[] =
		{
			"\"Q: \\u0051\"",
			"\"foo\"",
			"\n\"foo\"\n",
			" \"foo\" ",
			" \"foo\" err",
			"\"tab\\ttab\"",
			"\n\"tab\\ttab\"\n",
			" \"tab\\ttab\" ",
			" \"tab\\ttab\" err",
			"\"LF\\nLF\"",
			"\n\"LF\\nLF\"\n",
			" \"LF\\nLF\" ",
			" \"LF \\n LF\" ",
			" \"LF\\nLF\" err",
			"\"a\\tb\\nc\\td\"",
			"\n\"a\\tb\\nc\\td\"\n",
			" \"a\\tb\\nc\\td\" ",
			" \"a \\tb\\nc\\t d\" ",
			" \"a\\tb\\nc\\td\" err",
			"\"Q: \\u0051\"",
			"\"Omega: \\u03A9\"",
			"\"1 g clef 2 g clef 3: 1\\uD834\\uDD1E2\\uD834\\uDD1E3\"",
			"\"Error: \\u123\"",
			"\"Error: \\u123 \"",
			"\"Error: \\uZOOL\"",
			"42",
			"\n42\n",
			" 42 ",
			" 42 err",
			"-42",
			"\n-42\n",
			" -42 ",
			" -42 err",
			"42e2",
			"\n42e2\n",
			" 42e2 ",
			" 42e2 err",
			"-42e2",
			"\n-42e2\n",
			" -42e2 ",
			" -42e2 err",
			"42e-2",
			"\n42e-2\n",
			" 42e-2 ",
			" 42e-2 err",
			"-42e-2",
			"\n-42e-2\n",
			" -42e-2 ",
			" -42e-2 err",
			"42.42",
			"\n42.42\n",
			" 42.42 ",
			" 42.42 err",
			"-42.42",
			"\n-42.42\n",
			" -42.42 ",
			" -42.42 err",
			"42.42e2",
			"\n42.42e2\n",
			" 42.42e2 ",
			" 42.42e2 err",
			"-42.42e2",
			"\n-42.42e2\n",
			" -42.42e2 ",
			" -42.42e2 err",
			"42.42e-2",
			"\n42.42e-2\n",
			" 42.42e-2 ",
			" 42.42e-2 err",
			"-42.42e-2",
			"\n-42.42e-2\n",
			" -42.42e-2 ",
			" -42.42e-2 err",
			"true",
			"\ntrue\n",
			" true ",
			" true err",
			"false",
			"\nfalse\n",
			" false ",
			" false err",
			"null",
			"\nnull\n",
			" null ",
			" null err"
		};
		for (const auto& test : tests)
		{
			std::cout << "----Test-------------------" << std::endl;
			std::cout << test;
			try
			{
				std::istringstream testStream{ test };
				std::cout << "\n----Parsing----------------" << std::endl;
				neolib::json json{ testStream };
				std::cout << "\n----Result-----------------" << std::endl;
				json.write(std::cout);
			}
			catch (std::exception& e)
			{
				std::cout << "\n****Parse Error***********" << std::endl;
				std::cerr << e.what() << std::endl;
			}
			std::cout << "---------------------------" << std::endl;
		}

		std::string input;
		if (argc < 2)
		{
			std::cout << "Input: ";
			std::cin >> input;
		}
		else
			input = argv[1];

		neolib::json json{ input };

		std::string output;
		if (argc < 3)
		{
			std::cout << "Output: ";
			std::cin >> output;
		}
		else
			output = argv[2];

		json.write(output);
	}
	catch (std::exception& e)
	{
		std::cerr << "\nError: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

