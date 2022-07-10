#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <algorithm>
#include <chrono>

struct Chromosome
{
	Chromosome(const std::string& genes)
	{
		Genes = genes;
	}

	Chromosome(const Chromosome& chromosome)
	{
		Genes = chromosome.Genes;
		Fitness = chromosome.Fitness;
		Normalized_fitness = chromosome.Normalized_fitness;
		Accumulated_fitness = chromosome.Accumulated_fitness;
	}

	Chromosome Crossover(const Chromosome& other, float mutation_rate)
	{
		std::string new_genes;
		new_genes.resize(Genes.size());

		for (int i = 0; i < Genes.size(); i++)
		{
			float rnd = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); 

			if (rnd <= mutation_rate)
			{
				new_genes[i] = static_cast<char>(32 + rand() % 95);
			}
			else if (rnd <= (mutation_rate + ((1.0f - mutation_rate) / 2.0f)))
			{
				new_genes[i] = Genes[i];
			}
			else
			{
				new_genes[i] = other.Genes[i];
			}
		}

		return (new_genes);
	}

	friend std::ostream& operator<<(std::ostream& stream, const Chromosome& data);

	std::string Genes;
	float Fitness = 0;
	float Normalized_fitness = 0.0f;
	float Accumulated_fitness = 0.0f;
};

std::ostream& operator<<(std::ostream& stream, const Chromosome& data)
{
	stream << "Genes: " << data.Genes << " | fitness: " << std::setfill(' ') << std::setw(8) << data.Fitness;
	return (stream);
}

Chromosome Select(const std::vector<Chromosome>& population)
{
	float rnd = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);

	for (int i = 0; i < population.size(); i++)
	{
		if (population[i].Accumulated_fitness >= rnd)
		{
			return (population[i]);
		}
	}

	return (population[population.size() - 1]);
}


int main(int argc, char* argv[])
{	
	// Settings
	std::string input_phrase;
	int population_size = 10;
	float mutation_percentage = 0.07f;
	bool printing = true;

	assert(argc >= 2);

	input_phrase = argv[1];

	if (argc > 2)
	{
		population_size = std::stoi(argv[2]);
	}

	if (argc > 3)
	{
		mutation_percentage = std::stof(argv[3]);
	}

	if (argc > 4)
	{
		if (argv[4] == std::string("-noprint"))
		{
			printing = false;
		}
	}

	std::cout << "Input phrase: \"" << input_phrase << "\" length: " << input_phrase.length() << std::endl;
	std::cout << "Population size: " << population_size << std::endl;
	std::cout << "Mutation chance: " << mutation_percentage << std::endl;
	std::cout << "Printing mid execution: " << (printing ? "true" : "false") << std::endl;

	float fitness_increase = 9.5;
	float fitness_correct_multiplier = 1.0f;
	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> end_time;
	int generation_count = 1;
	

	std::srand(static_cast<unsigned int>(time(nullptr)));

	std::vector<Chromosome> population;
	// Random generate the first generation
	for (size_t i = 0; i < population_size; i++)
	{
		std::string new_genes;
		new_genes.resize(input_phrase.length());

		for (size_t j = 0; j < input_phrase.length(); j++)
		{
			new_genes[j] = static_cast<char>(32 + rand() % 95);
		}

		population.emplace(population.begin() + i, new_genes);
	}

	// Start the algorithm 
	bool found = false;
	while (true)
	{
		// Calculate individual fitness and track the sum of fitness values for the next step
		float total_fitness = 0;
		for (auto& chromosome : population)
		{
			float fitness = 0;
			int i = 0;

			for (auto value : chromosome.Genes)
			{
				float delta = static_cast<float>(std::abs(value - input_phrase[i]));

				//if (delta == 0)
				//{
				//	fitness += fitness_increase * fitness_correct_multiplier;
				//}
				//else
				//{
				//	fitness += fitness_increase * (delta/95);
				//}

				fitness += fitness_increase * (95 - (delta / 95));

				++i;
			}

			total_fitness += fitness;
			chromosome.Fitness = fitness;
		}

		// Normalize fitness and accumulate it
		// Pick the fittest individual for display
		Chromosome& greatest = population[0];
		float accumulated_fitness = 0.0;
		for (auto& chromosome : population)
		{
			chromosome.Normalized_fitness = static_cast<float>(chromosome.Fitness) / total_fitness;
			chromosome.Accumulated_fitness = accumulated_fitness + chromosome.Normalized_fitness;
			accumulated_fitness = chromosome.Accumulated_fitness;

			if (greatest.Normalized_fitness < chromosome.Normalized_fitness)
			{
				greatest = chromosome;
			}
		}
		
		if (found)
		{
			// The correct phrase was found in the previous generation
			// Allow the program to calculate fitness values for the generations and break right after
			// So the values can be printed out
			break;
		}
		
		if (printing && generation_count % 5 == 1)
		{
			std::cout << "Generation: " << generation_count - 1 << ". Fittest individual: " << greatest << std::endl;
		}

		// Create the next generation from the current one
		std::vector<Chromosome> new_population;
		for(int i = 0; i < population_size; i++)
		{
			Chromosome child = Select(population).Crossover(Select(population), mutation_percentage);

			if (input_phrase == child.Genes)
			{
				found = true;
			}

			new_population.emplace_back(child);
		}

		// Replace the old population
		population = new_population;
		generation_count++;
	}

	end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> duration = end_time - start_time;

	std::cout << std::endl << "RESULTS" << std::endl << std::endl;
	std::cout << "Last population" << std::endl;
	
	for (auto& individual : population)
	{
		std::cout << individual << " | Normalized against input: ";
		std::cout << std::fixed << std::setprecision(3) << individual.Fitness / (input_phrase.length() * fitness_increase * fitness_correct_multiplier);
		std::cout << (individual.Genes == input_phrase ? " <<<<" : "") << std::endl;
	}

	std::cout << "Execution time: " << duration.count() << "ms" << std::endl;
	std::cout << "Generations passed: " << generation_count << std::endl;
	return (0);
}