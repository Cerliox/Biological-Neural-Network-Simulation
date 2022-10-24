#include "BioSimulation.h"

Organism::Organism() {

}
void Organism::Init(BioSimulation* sim) {
	x = Random::RandomInt(0, sim->max_x);
	y = Random::RandomInt(0, sim->max_y);
	speed = 0.0;

	energy = sim->start_energy;
	health = sim->start_health;

	this->sim = sim;
	this->brain.organism = this;

	// Neurons
	this->brain.input = new Inputneuron[SENSOR_SIZE];
	for (int i = 0; i < SENSOR_SIZE; i++)
		this->brain.input[i].sensor = (Sensor) i;

	this->brain.output = new Outputneuron[ACTION_SIZE];
	for (int i = 0; i < ACTION_SIZE; i++)
		this->brain.output[i].action = (Action)i;

	Connection* connections;
	if (this->sim->start_hidden_neurons != 0) {
		this->brain.hidden.reserve(this->sim->start_hidden_neurons);
		for (int i = 0; i < this->sim->start_hidden_neurons; i++)
			this->brain.hidden.emplace_back(Hiddenneuron());

		// Connections
		//	Input -> Hidden
		connections = new Connection[this->sim->start_ih_connections];
		for (int i = 0; i < this->sim->start_ih_connections; i++) {
			connections[i].in = Random::RandomInt(0, SENSOR_SIZE);
			connections[i].out = Random::RandomInt(0, this->brain.hidden.size());
			connections[i].weight = Random::RandomDouble(-1.0f, -1.0f);
		}
		for (int i = 0; i < this->sim->start_ih_connections; i++) {
			bool added = false;
			for (int a = 0; a < this->brain.input_to_hidden.size(); a++) {
				if (this->brain.input_to_hidden[a] == connections[i]) {
					this->brain.input_to_hidden[a].weight += connections[i].weight;
					added = true;
					continue;
				}
			}
			if (!added) {
				this->brain.input_to_hidden.push_back(connections[i]);
			}
		}
		delete[] connections;

		// Hidden -> Hidden
		connections = new Connection[this->sim->start_hh_connections];
		for (int i = 0; i < this->sim->start_hh_connections; i++) {
			connections[i].in = Random::RandomInt(0, this->brain.hidden.size());
			connections[i].out = Random::RandomInt(0, this->brain.hidden.size());
			connections[i].weight = Random::RandomDouble(-1.0f, -1.0f);
		}
		for (int i = 0; i < this->sim->start_hh_connections; i++) {
			bool added = false;
			for (int a = 0; a < this->brain.hidden_to_hidden.size(); a++) {
				if (this->brain.hidden_to_hidden[a] == connections[i]) {
					this->brain.hidden_to_hidden[a].weight += connections[i].weight;
					added = true;
					continue;
				}
			}
			if (!added) {
				this->brain.hidden_to_hidden.push_back(connections[i]);
			}
		}
		delete[] connections;

		// Hidden -> Output
		connections = new Connection[this->sim->start_ho_connections];
		for (int i = 0; i < this->sim->start_ho_connections; i++) {
			connections[i].in = Random::RandomInt(0, this->brain.hidden.size());
			connections[i].out = Random::RandomInt(0, ACTION_SIZE);
			connections[i].weight = Random::RandomDouble(-1.0f, -1.0f);
		}
		for (int i = 0; i < this->sim->start_ho_connections; i++) {
			bool added = false;
			for (int a = 0; a < this->brain.hidden_to_output.size(); a++) {
				if (this->brain.hidden_to_output[a] == connections[i]) {
					this->brain.hidden_to_output[a].weight += connections[i].weight;
					added = true;
					continue;
				}
			}
			if (!added) {
				this->brain.hidden_to_output.push_back(connections[i]);
			}
		}
		delete[] connections;
	}
	// Input -> Output
	connections = new Connection[this->sim->start_io_connections];
	for (int i = 0; i < this->sim->start_io_connections; i++) {
		connections[i].in = Random::RandomInt(0, SENSOR_SIZE);
		connections[i].out = Random::RandomInt(0, ACTION_SIZE);
		connections[i].weight = Random::RandomDouble(-1.0f, -1.0f);
	}
	for (int i = 0; i < this->sim->start_io_connections; i++) {
		bool added = false;
		for (int a = 0; a < this->brain.input_to_output.size(); a++) {
			if (this->brain.input_to_output[a] == connections[i]) {
				this->brain.input_to_output[a].weight += connections[i].weight;
				added = true;
				continue;
			}
		}
		if (!added) {
			this->brain.input_to_output.push_back(connections[i]);
		}
	}
	delete[] connections;
	
}

void Organism::Update() {
	this->brain.Behave();

	speed = fmax(speed, this->sim->max_speed);

	if (direction.x != 0.0 && direction.y != 0.0 && speed != 0.0 && energy > speed) {
		this->direction.Normalize();

		Vec2 add = direction * speed;

		this->energy -= speed;

		this->x += (int)add.x;
		this->y += (int)add.y;
	}
	if (this->energy < this->sim->max_energy) {
		this->energy += this->sim->organism_energy_refreshrate;
	}
	this->energy = fmax(this->energy, this->sim->max_energy);

	if (this->x < 0)
		this->x = 0;
	else if (this->x > this->sim->max_x - this->sim->organism_width)
		this->x = this->sim->max_x - this->sim->organism_width;
	if (this->y < 0)
		this->y = 0;
	else if (this->y > this->sim->max_y - this->sim->organism_height)
		this->y = this->sim->max_y - this->sim->organism_height;
}