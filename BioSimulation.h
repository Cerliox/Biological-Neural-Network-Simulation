﻿#pragma once

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

#define EULER 2.7182818284590452353602874

using namespace cimg_library;

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
	DFX,
	DFY,
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

	Inputneuron input[SENSOR_SIZE];
	std::vector<Hiddenneuron> hidden;
	Outputneuron output[ACTION_SIZE];

	std::vector<Connection> input_to_hidden;
	std::vector<Connection> hidden_to_hidden;
	std::vector<Connection> input_to_output;
	std::vector<Connection> hidden_to_output;

	void Behave();
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
	double duplication = 0.0;

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

	int max_x, max_y, max_organisms, max_food;
	double max_energy, max_speed, max_health;
	double max_duplication;
	
	int start_organisms, start_health, start_energy;
	int start_hidden_neurons, start_ih_connections, start_hh_connections, start_ho_connections, start_io_connections;
	int start_food;

	int organism_width, organism_height;
	int organism_color[3];
	int organism_duplication_amount;
	double organism_energy_refreshrate;

	double (*brain_input_activation_function)(double);
	double (*brain_hidden_activation_function)(double);
	double (*brain_output_activation_function)(double);

	double food_per_iteration;
	double food_refresh;
	double foodcounter = 0.0;

	double health_loss_rate;
	
	bool save_video;
	int save_length;
	int save_fps;

	int video_sleep;

	double mutation_add_hidden;
	double mutation_add_weight;
	double mutation_weight;
	double mutation_bias;

	std::vector<Organism*> organisms;
	std::vector<Food*> food;

	// Init
	BioSimulation(String);

	// Running
	void Update(); // TODO: Update

	// Output 
	Image CreateImage(); // TODO: Implement this
};


