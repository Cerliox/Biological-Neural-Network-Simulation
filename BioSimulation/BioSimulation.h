#pragma once

#include <iostream>
#include "CImg.h"
#include <string>
#include <vector>
#include "INIReader.h"
#include "Random.h"
#include "Vector.h"

#define Image CImg<unsigned char>
#define String std::string
#define Map std::map

using namespace cimg_library;

// Defautl classes and structs
struct Organism;
struct BioSimulation;
struct Brain;
struct Inputneuron;
struct Hiddenneuron;
struct Outputneuron;

enum Sensor {
	HEALTH, // Get health
	ENERGY, // Get energy
	DWX, // Distance to closest wall x
	DWY, // Distance to closest wall y
	DCEX, // Distance to closest organism x
	DCEY, // Distance to closest organism y

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

struct Inputneuron {
	Sensor sensor;
	double bias = 0.0;
	double value = 0.0;

	void GetValue(Brain*);
};
struct Hiddenneuron {
	double bias = 0.0;
	double value = 0.0;

	void Clear();
};
struct Outputneuron {
	Action action;
	double bias = 0.0;
	double value = 0.0;

	void Clear();
	void TranslateValue(Organism*);
};

struct Connection {
	int in, out;
	double weight = 1.0;
	bool operator==(Connection&);
};

struct Brain {
	Organism* organism;

	Brain();

	Inputneuron* input;
	std::vector<Hiddenneuron> hidden;
	Outputneuron* output;

	std::vector<Connection> input_to_hidden;
	std::vector<Connection> hidden_to_hidden;
	std::vector<Connection> input_to_output;
	std::vector<Connection> hidden_to_output;

	void Behave();
};


struct Organism {
	BioSimulation* sim;

	int x, y;
	Vec2 direction;
	double speed;
	double health;
	double energy;

	Brain brain;

	Organism();
	
	void Init(BioSimulation*); // Randomize organism

	void Inherit(Organism*);

	// What should they be able to do
	void Update();
};

struct BioSimulation {
	INIReader* config_reader;

	int max_x, max_y, max_organisms;
	double max_energy, max_speed;
	
	int start_organisms, start_health, start_energy;
	int start_hidden_neurons, start_ih_connections, start_hh_connections, start_ho_connections, start_io_connections;

	int organism_width, organism_height;
	int organism_color[3];
	double organism_energy_refreshrate;

	double threshold_speed;

	std::vector<Organism*> organisms;
	
	// Init
	BioSimulation(String);

	// Running
	void Update(); // TODO: Update

	// Output 
	Image CreateImage(); // TODO: Implement this
};


