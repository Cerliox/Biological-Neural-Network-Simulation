#include "BioSimulation.h"

Organism::Organism() {
	this->brain.organism = this;
}
void Organism::Init(BioSimulation* sim) {
	x = Random::RandomInt(0, sim->max_x);
	y = Random::RandomInt(0, sim->max_y);

	energy = sim->start_energy;
	health = sim->start_health;

	this->sim = sim;

	// Neurons
	for (int i = 0; i < SENSOR_SIZE; i++)
		this->brain.input[i].sensor = (Sensor) i;

	for (int i = 0; i < ACTION_SIZE; i++)
		this->brain.output[i].action = (Action) i;

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
			connections[i].weight = Random::RandomDouble(-1.0f, 1.0f);
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
			connections[i].weight = Random::RandomDouble(-1.0f, 1.0f);
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
			connections[i].weight = Random::RandomDouble(-1.0f, 1.0f);
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
		connections[i].weight = Random::RandomDouble(-1.0f, 1.0f);
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
	// Closest organism
	double cd = -1.0;
	for (int i = 0; i < this->sim->organisms.size(); i++) {
		Organism* o = this->sim->organisms[i];
		if (o == this)
			continue;
		Vec2 dv = Vec2(this->x - o->x, this->y - o->y);
		double d = dv.Length();
		if (d < cd || cd == -1.0) {
			cd = d;
			this->closest = o;
			this->distance_to_closest = dv;
		}
	}
	// Closest food
	cd = -1.0;
	for (int i = 0; i < this->sim->food.size(); i++) {
		Food* o = this->sim->food[i];
		Vec2 dv = Vec2(this->x - o->x, this->y - o->y);
		double d = dv.Length();
		if (d < cd || cd == -1.0) {
			cd = d;
			this->closest_food = o;
			this->distance_to_closest_food = dv;
		}
	}

	this->brain.Behave();

	speed = fminmax(speed, -this->sim->max_speed, this->sim->max_speed);

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

	bool eat = false;
	for (int i = 0; i < sim->food.size(); i++) {
		if (CollisionDetection::Collide(this, sim->food[i])) {
			this->energy += sim->food_refresh;
			this->health += sim->food_refresh;
			this->duplication += sim->food_refresh;
			sim->food.erase(sim->food.begin() + i);
			eat = true;
			i--;
		}
	}
	if (!eat) {
		this->health -= sim->health_loss_rate;
	}

	this->energy = fminmax(this->energy, 0.0, this->sim->max_energy);
	this->health = fmin(this->health, sim->max_health);

	if (this->x < 0)
		this->x = 0;
	else if (this->x > this->sim->max_x - this->sim->organism_width)
		this->x = this->sim->max_x - this->sim->organism_width;
	if (this->y < 0)
		this->y = 0;
	else if (this->y > this->sim->max_y - this->sim->organism_height)
		this->y = this->sim->max_y - this->sim->organism_height;
}

void Organism::Inherit(Organism* parent) {
	this->sim = parent->sim;
	
	this->x = parent->x;
	this->y = parent->y;

	this->health = sim->max_health;
	this->energy = sim->max_energy;
	this->duplication = 0.0;

	// Brain-copying
	memcpy(this->brain.input, parent->brain.input, sizeof(Inputneuron) * SENSOR_SIZE);
	memcpy(this->brain.output, parent->brain.output, sizeof(Inputneuron) * ACTION_SIZE);
	this->brain.hidden = parent->brain.hidden;
	
	this->brain.input_to_hidden = parent->brain.input_to_hidden;
	this->brain.hidden_to_hidden = parent->brain.hidden_to_hidden;
	this->brain.hidden_to_output = parent->brain.hidden_to_output;
	this->brain.input_to_output = parent->brain.input_to_output;
	
	// Mutations
	// Add hidden
	double c_add_hidden = Random::RandomDouble(0.0, 1.0);
	if (c_add_hidden <= sim->mutation_add_hidden) {
		this->brain.hidden.push_back(Hiddenneuron());
	}

	// Bias
	for (int i = 0; i < SENSOR_SIZE; i++) {
		brain.input[i].bias += Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
	}
	for (int i = 0; i < brain.hidden.size(); i++) {
		brain.hidden[i].bias += Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
	}
	for (int i = 0; i < ACTION_SIZE; i++) {
		brain.output[i].bias += Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
	}

	// Weights
	for (int i = 0; i < this->brain.input_to_hidden.size(); i++) {
		Connection* c = &this->brain.input_to_hidden[i];
		c->weight += Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
	}
	for (int i = 0; i < this->brain.hidden_to_hidden.size(); i++) {
		Connection* c = &this->brain.hidden_to_hidden[i];
		c->weight += Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
	}
	for (int i = 0; i < this->brain.hidden_to_output.size(); i++) {
		Connection* c = &this->brain.hidden_to_output[i];
		c->weight += Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
	}
	for (int i = 0; i < this->brain.input_to_output.size(); i++) {
		Connection* c = &this->brain.input_to_output[i];
		c->weight += Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
	}

	// Add Connection
	double c_add_weight = Random::RandomDouble(0.0, 1.0);
	while (c_add_weight <= sim->mutation_add_weight) {
		int a = Random::RandomInt(0, 4);

		Connection c;
		if (a == 0) { // Input -> Hidden
			c.in = Random::RandomInt(0, SENSOR_SIZE);
			c.out = Random::RandomInt(0, brain.hidden.size());
			c.weight = Random::RandomDouble(-1.0, 1.0);
			brain.input_to_hidden.push_back(c);
		}
		else if (a == 1) { // Hidden -> Hidden
			c.in = Random::RandomInt(0, brain.hidden.size());
			c.out = Random::RandomInt(0, brain.hidden.size());
			c.weight = Random::RandomDouble(-1.0, 1.0);
			brain.hidden_to_hidden.push_back(c);
		}
		else if (a == 2) { // Hidden -> Output
			c.in = Random::RandomInt(0, brain.hidden.size());
			c.out = Random::RandomInt(0, ACTION_SIZE);
			c.weight = Random::RandomDouble(-1.0, 1.0);
			brain.hidden_to_output.push_back(c);
		}
		else if (a == 3) { // Input -> Output
			c.in = Random::RandomInt(0, SENSOR_SIZE);
			c.out = Random::RandomInt(0, ACTION_SIZE);
			c.weight = Random::RandomDouble(-1.0, 1.0);
			brain.input_to_output.push_back(c);
		}

		c_add_weight = Random::RandomDouble(0.0, 1.0);
	}
}