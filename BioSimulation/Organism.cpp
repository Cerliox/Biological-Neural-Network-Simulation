#include "BioSimulation.h"

Organism::Organism() {
	this->brain.organism = this;
}
void Organism::Init(BioSimulation* sim) {
	x = Random::RandomInt(0, sim->max_x);
	y = Random::RandomInt(0, sim->max_y);

	energy = sim->start_energy;
	health = sim->start_health;

	color[0] = Random::RandomInt(0, 255);
	color[1] = Random::RandomInt(0, 255);
	color[2] = Random::RandomInt(0, 255);

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
	this->brain.ComputeComplexity();
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

	speed = fmin(speed, this->sim->max_speed);
	if (direction.x != 0.0 && direction.y != 0.0 && speed > 0.0 && energy > 0.0) {
		this->direction.Normalize();
		
		Vec2 add = direction * speed;

		this->energy -= pow(speed, 2) * sim->organism_energy_loss_speed_multiplier;

		this->x += (int)add.x;
		this->y += (int)add.y;
	}
	this->health -= sim->health_loss_rate;
	energy -= this->brain.complexity * sim->organism_energy_loss_complexity_multiplier;
	if (energy <= 0.0) {
		this->health -= sim->health_loss_rate;
	}
	this->energy += sim->organism_energy_refreshrate;

	bool eat = false;
	for (int i = 0; i < sim->food.size(); i++) {
		if (CollisionDetection::Collide(this, sim->food[i])) {
			this->energy += sim->food_refresh;
			this->health += sim->food_refresh;
			this->replication += sim->food_refresh;
			Food* f = sim->food[i];
			delete f;
			sim->food.erase(sim->food.begin() + i);
			eat = true;
			i--;
		}
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

	double mutation_color_change = 0.0;

	this->health = sim->max_health;
	this->energy = sim->max_energy;
	this->replication = 0.0;
	this->amount_of_replications = 0;

	// Brain-copying
	memcpy(this->brain.input, parent->brain.input, sizeof(Inputneuron) * SENSOR_SIZE);
	memcpy(this->brain.output, parent->brain.output, sizeof(Inputneuron) * ACTION_SIZE);
	this->brain.hidden = parent->brain.hidden;
	
	this->brain.input_to_hidden = parent->brain.input_to_hidden;
	this->brain.hidden_to_hidden = parent->brain.hidden_to_hidden;
	this->brain.hidden_to_output = parent->brain.hidden_to_output;
	this->brain.input_to_output = parent->brain.input_to_output;
	
	// Mutations
	// Remove hidden
	/*
	Here is an error cause if we are removing a neuron every connection connects to one neuron after the deleted neuron.
	Is should do a new neuron system where connections don't hold the index but rather a pointer to the neuron. For that i need an overall neuron class
	*/
	double c_remove_hidden = Random::RandomDouble(0.0, 1.0);
	if (c_remove_hidden <= sim->mutation_remove_hidden && brain.hidden.size() >= 1) {
		int index = Random::RandomInt(0, brain.hidden.size());
		for (int i = 0; i < brain.input_to_hidden.size(); i++) {
			if (brain.input_to_hidden[i].in == index || brain.input_to_hidden[i].out == index)
				brain.input_to_hidden.erase(brain.input_to_hidden.begin() + i);
		}
		for (int i = 0; i < brain.hidden_to_hidden.size(); i++) {
			if (brain.hidden_to_hidden[i].in == index || brain.hidden_to_hidden[i].out == index)
				brain.hidden_to_hidden.erase(brain.hidden_to_hidden.begin() + i);
		}
		for (int i = 0; i < brain.hidden_to_output.size(); i++) {
			if (brain.hidden_to_output[i].in == index || brain.hidden_to_output[i].out == index)
				brain.hidden_to_output.erase(brain.hidden_to_output.begin() + i);
		}
		brain.hidden.erase(brain.hidden.begin() + index);
		mutation_color_change += sim->mutation_color_change_hidden;
	}
	// Add hidden
	double c_add_hidden = Random::RandomDouble(0.0, 1.0);
	if (c_add_hidden <= sim->mutation_add_hidden) {
		this->brain.hidden.push_back(Hiddenneuron());
		mutation_color_change += sim->mutation_color_change_hidden;
	}

	// Bias
	for (int i = 0; i < SENSOR_SIZE; i++) {
		double d = Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
		brain.input[i].bias += d;
		mutation_color_change += abs(d);
	}
	for (int i = 0; i < brain.hidden.size(); i++) {
		double d = Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
		brain.hidden[i].bias += d;
		mutation_color_change += abs(d);
	}
	for (int i = 0; i < ACTION_SIZE; i++) {
		double d = Random::RandomDouble(-sim->mutation_bias, sim->mutation_bias);
		brain.output[i].bias += d;
		mutation_color_change += abs(d);
	}

	// Weights
	for (int i = 0; i < this->brain.input_to_hidden.size(); i++) {
		Connection* c = &this->brain.input_to_hidden[i];
		double d = Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
		c->weight += d;
		mutation_color_change += abs(d);
	}
	for (int i = 0; i < this->brain.hidden_to_hidden.size(); i++) {
		Connection* c = &this->brain.hidden_to_hidden[i];
		double d = Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
		c->weight += d;
		mutation_color_change += abs(d);
	}
	for (int i = 0; i < this->brain.hidden_to_output.size(); i++) {
		Connection* c = &this->brain.hidden_to_output[i];
		double d = Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
		c->weight += d;
		mutation_color_change += abs(d);
	}
	for (int i = 0; i < this->brain.input_to_output.size(); i++) {
		Connection* c = &this->brain.input_to_output[i];
		double d = Random::RandomDouble(-sim->mutation_weight, sim->mutation_weight);
		c->weight += d;
		mutation_color_change += abs(d);
	}

	// Add Connection
	double c_add_weight = Random::RandomDouble(0.0, 1.0);
	while (c_add_weight <= sim->mutation_add_weight) {
		Connection c;
		mutation_color_change += sim->mutation_color_change_weight;
		if (brain.hidden.size() != 0) {
			int a = Random::RandomInt(0, 4);

			if (a == 0 && brain.hidden.size() != 0) { // Input -> Hidden
				c.in = Random::RandomInt(0, SENSOR_SIZE);
				c.out = Random::RandomInt(0, brain.hidden.size());
				c.weight = Random::RandomDouble(-1.0, 1.0);
				bool added = false;
				for (int i = 0; i < brain.input_to_hidden.size(); i++) {
					if (brain.input_to_hidden[i] == c) {
						brain.input_to_hidden[i].weight += c.weight;
						added = true;
					}
				}
				if(!added)
					brain.input_to_hidden.push_back(c);
			}
			else if (a == 1 && brain.hidden.size() != 0) { // Hidden -> Hidden
				c.in = Random::RandomInt(0, brain.hidden.size());
				c.out = Random::RandomInt(0, brain.hidden.size());
				c.weight = Random::RandomDouble(-1.0, 1.0);
				bool added = false;
				for (int i = 0; i < brain.hidden_to_hidden.size(); i++) {
					if (brain.hidden_to_hidden[i] == c) {
						brain.hidden_to_hidden[i].weight += c.weight;
						added = true;
					}
				}
				if (!added)
					brain.hidden_to_hidden.push_back(c);
			}
			else if (a == 2 && brain.hidden.size() != 0) { // Hidden -> Output
				c.in = Random::RandomInt(0, brain.hidden.size());
				c.out = Random::RandomInt(0, ACTION_SIZE);
				c.weight = Random::RandomDouble(-1.0, 1.0);
				bool added = false;
				for (int i = 0; i < brain.hidden_to_output.size(); i++) {
					if (brain.hidden_to_output[i] == c) {
						brain.hidden_to_output[i].weight += c.weight;
						added = true;
					}
				}
				if (!added)
					brain.hidden_to_output.push_back(c);
			}
			else if (a == 3) { // Input -> Output
				c.in = Random::RandomInt(0, SENSOR_SIZE);
				c.out = Random::RandomInt(0, ACTION_SIZE);
				c.weight = Random::RandomDouble(-1.0, 1.0);
				bool added = false;
				for (int i = 0; i < brain.input_to_output.size(); i++) {
					if (brain.input_to_output[i] == c) {
						brain.input_to_output[i].weight += c.weight;
						added = true;
					}
				}
				if (!added)
					brain.input_to_output.push_back(c);
			}
		}
		else {
			c.in = Random::RandomInt(0, SENSOR_SIZE);
			c.out = Random::RandomInt(0, ACTION_SIZE);
			c.weight = Random::RandomDouble(-1.0, 1.0);
			bool added = false;
			for (int i = 0; i < brain.input_to_output.size(); i++) {
				if (brain.input_to_output[i] == c) {
					brain.input_to_output[i].weight += c.weight;
					added = true;
				}
			}
			if (!added)
				brain.input_to_output.push_back(c);
		}

		c_add_weight = Random::RandomDouble(0.0, 1.0);
	}
	mutation_color_change = fminmax(mutation_color_change * sim->mutation_color_change_multiplier, 0.0, sim->mutation_color_max_change);
	double r = Random::RandomInt(-mutation_color_change, mutation_color_change);
	if (parent->color[0] + r > 255 || parent->color[0] + r < 0)
		r = -r;
	this->color[0] = fminmax(parent->color[0] + r, 0, 255);
	mutation_color_change -= abs(r);
	mutation_color_change = fmax(0.0, mutation_color_change);

	r = Random::RandomInt(-mutation_color_change, mutation_color_change);
	if (parent->color[1] + r > 255 || parent->color[0] + r < 0)
		r = -r;
	this->color[1] = fminmax(parent->color[1] + r, 0, 255);
	mutation_color_change -= abs(r);
	mutation_color_change = fmax(0.0, mutation_color_change);

	r = Random::RandomInt(-mutation_color_change, mutation_color_change);
	if (parent->color[2] + r > 255 || parent->color[0] + r < 0)
		r = -r;
	this->color[2] = fminmax(parent->color[2] + r, 0, 255);

	this->brain.ComputeComplexity();
}