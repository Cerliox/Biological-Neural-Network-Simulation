#pragma once

#include <iostream>
#include "CImg.h"
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include "INIReader.h"
#include "Random.h"
#include "Vector.h"
#include <direct.h>

#define Image CImg<unsigned char>
#define String std::string
#define Map std::map

#define EULER 2.7182818284590452353602874

using namespace cimg_library;

struct Clock {
private:
	std::chrono::high_resolution_clock::time_point start;
public:
	void Start() {
		start = std::chrono::high_resolution_clock::now();
	}
	long long ElapsedMicroseconds() {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
	long long ElapsedMilliseconds() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
	long long ElapsedNanoseconds() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
	long long ElapsedSeconds() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
};

// Defautl classes and structs
struct Organism;
struct Food;
struct BioSimulation;
struct Brain;
struct Inputneuron;
struct Hiddenneuron;
struct Outputneuron;

namespace ActivationFunctions {
	double ReLU(double);
	double Linear(double);
	double Sigmoid(double);
	
	void GetActivationFunction(String, double (*&)(double));
}
namespace CollisionDetection {
	bool Collide(Organism*, Food*);
}

enum Sensor {
	HEALTH, // Get health
	ENERGY, // Get energy
	PX, // Position x from -0.5 -> 0.5
	PY, // Position y y -0.5 -> 0.5
	DFX, // Distance to closest food x
	DFY, // Distance to closest food y
	/*DCEX, // Distance to closest organism x
	DCEY, // Distance to closest organism */

	/*PH1, // Pheremone channel 1
	PH1l, // Latest pheremone channel 1
	PH2, // Pheremone channel 2
	PH2l, // Latest pheremone channel 2
	PH3, // Pheremone channel 3
	PH3l // Latest pheremone channel 3
	*/
	SENSOR_SIZE
};
enum Action {
	MOVEX,
	MOVEY,
	MOVESPEED,
	ACTION_SIZE
};

struct Neuron {
	Sensor sensor;
	Action action;
	double bias = 0.0;
	double value = 0.0;

	void Clear();
};

struct Connection {
	Neuron* in;
	Neuron* out;
	double weight = 1.0;
	bool operator==(Connection&);
};

struct Brain {
	Organism* organism;

	Brain();

	Neuron input[SENSOR_SIZE];
	std::vector<Neuron*> hidden;
	Neuron output[ACTION_SIZE];

	std::vector<Connection> input_to_hidden;
	std::vector<Connection> hidden_to_hidden;
	std::vector<Connection> input_to_output;
	std::vector<Connection> hidden_to_output;

	double complexity = 0.0;

	Neuron* GetRandomInputneuron();
	Neuron* GetRandomHiddenneuron();
	Neuron* GetRandomHiddenneuron(int&);
	Neuron* GetRandomOutputneuron();

	int FindInput(Neuron*);
	int FindHidden(Neuron*);
	int FindOutput(Neuron*);

	void Behave();
	void ComputeComplexity();

	void SaveToFile(std::ofstream&);
};

struct Food {
	BioSimulation* sim;

	int x, y;
};

struct Organism {
	BioSimulation* sim;

	int x, y;
	Vec2 direction = Vec2(0.0);
	double speed = 0.0;
	double health;
	double energy;
	double replication = 0.0;
	unsigned int amount_of_replications = 0;

	char color[3];

	Brain brain;

	Vec2 distance_to_closest = Vec2(0.0);
	Organism* closest;
	
	Vec2 distance_to_closest_food = Vec2(0.0);
	Food* closest_food;

	Organism();
	
	void Init(BioSimulation*); // Randomize organism

	void Inherit(Organism*);

	// What should they be able to do
	void Update();
};

struct BioSimulation {
	INIReader* config_reader;
	String configfilename;
	int curr_iteration = 0;
	int curr_reset = 0;
	long iteration_time_ms;

	bool auto_reset;
	int auto_reset_only_one_max;
	int auto_reset_only_one_counter = 0;

	int max_x, max_y, max_organisms, max_food;
	double max_energy, max_speed, max_health;
	double max_replication;
	int max_hidden_neurons;

	int start_organisms, start_health, start_energy;
	int start_hidden_neurons, start_ih_connections, start_hh_connections, start_ho_connections, start_io_connections;
	int start_food;

	int organism_width, organism_height;
	int organism_duplication_amount;
	double organism_energy_refreshrate;
	double organism_energy_loss_complexity_multiplier;
	double organism_energy_loss_speed_multiplier;
	bool organism_failsafe;
	double organism_health_loss_rate;

	double (*brain_input_activation_function)(double);
	double (*brain_hidden_activation_function)(double);
	double (*brain_output_activation_function)(double);

	double food_per_iteration;
	double food_refresh;
	int food_color[3];
	double foodcounter = 0.0;
	
	bool display_simulation;
	int display_sleep;

	bool display_statistics;
	std::vector<int> display_statistics_organism_data;
	std::vector<int> display_statistics_food_data;
	int display_statistics_data_length;
	int display_statistics_size_x;
	int display_statistics_max_y;

	String save_folder;

	bool save_video;
	String save_video_filename;
	int save_length;
	int save_fps;

	bool save_last_brains;
	String save_last_brains_filename;

	bool save_statistics;
	bool save_extended_statistics;
	String save_statistics_filename;

	double mutation_add_hidden;
	double mutation_remove_hidden;
	double mutation_add_weight;
	double mutation_change_weight;
	double mutation_change_weight_rate;
	double mutation_change_bias;
	double mutation_change_bias_rate;
	double mutation_color_change_hidden;
	double mutation_color_change_weight;
	double mutation_color_max_change;
	double mutation_color_change_multiplier;
	int mutation_colorrate;

	std::vector<Organism*> organisms;
	std::vector<Food*> food;

	// Init
	BioSimulation(String);

	// Running
	void LoadConfig();
	void Spawn();
	void Update();
	void Reset();

	// Output 
	Image CreateImage(); // TODO: Implement this
	void WriteStatistics(std::ofstream&);
};


