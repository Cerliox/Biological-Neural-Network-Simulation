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
	
	this->brain.hidden.reserve(this->sim->max_hidden_neurons);

	Connection* connections;
	if (this->sim->start_hidden_neurons != 0) {
		for (int i = 0; i < this->sim->start_hidden_neurons; i++) {
			Neuron* n = new Neuron();
			this->brain.hidden.emplace_back(n);
		}

		// Connections
		//	Input -> Hidden
		connections = new Connection[this->sim->start_ih_connections];
		for (int i = 0; i < this->sim->start_ih_connections; i++) {
			connections[i].in = brain.GetRandomInputneuron();
			connections[i].out = brain.GetRandomHiddenneuron();
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
			connections[i].in = brain.GetRandomHiddenneuron();
			connections[i].out = brain.GetRandomHiddenneuron();
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
			connections[i].in = brain.GetRandomHiddenneuron();
			connections[i].out = brain.GetRandomOutputneuron();
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
		connections[i].in = brain.GetRandomInputneuron();
		connections[i].out = brain.GetRandomOutputneuron();
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
	this->health -= sim->organism_health_loss_rate;
	energy -= this->brain.complexity * sim->organism_energy_loss_complexity_multiplier;
	if (energy <= 0.0) {
		this->health -= sim->organism_health_loss_rate;
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
	memcpy(this->brain.input, parent->brain.input, sizeof(Neuron) * SENSOR_SIZE);
	memcpy(this->brain.output, parent->brain.output, sizeof(Neuron) * ACTION_SIZE);
	this->brain.hidden.reserve(sim->max_hidden_neurons);
	for (int i = 0; i < parent->brain.hidden.size(); i++) {
		Neuron* n = new Neuron();
		n->bias = parent->brain.hidden[i]->bias;
		this->brain.hidden.emplace_back(n);
	}
	
	this->brain.input_to_hidden.reserve(parent->brain.input_to_hidden.size());
	for (Connection c : parent->brain.input_to_hidden) {
		Connection c2;
		c2.in = &this->brain.input[parent->brain.FindInput(c.in)];
		c2.out = this->brain.hidden[parent->brain.FindHidden(c.out)];
		c2.weight = c.weight;
		this->brain.input_to_hidden.emplace_back(c2);
	}
	this->brain.hidden_to_hidden.reserve(parent->brain.hidden_to_hidden.size());
	for (Connection c : parent->brain.hidden_to_hidden) {
		Connection c2;
		c2.in = this->brain.hidden[parent->brain.FindHidden(c.in)];
		c2.out = this->brain.hidden[parent->brain.FindHidden(c.out)];
		c2.weight = c.weight;
		this->brain.hidden_to_hidden.emplace_back(c2);
	}
	this->brain.hidden_to_output.reserve(parent->brain.hidden_to_output.size());
	for (Connection c : parent->brain.hidden_to_output) {
		Connection c2;
		c2.in = this->brain.hidden[parent->brain.FindHidden(c.in)];
		c2.out = &this->brain.output[parent->brain.FindOutput(c.out)];
		c2.weight = c.weight;
		this->brain.hidden_to_output.emplace_back(c2);
	}
	this->brain.input_to_output.reserve(parent->brain.input_to_output.size());
	for (Connection c : parent->brain.input_to_output) {
		Connection c2;
		c2.in = &this->brain.input[parent->brain.FindInput(c.in)];
		c2.out = &this->brain.output[parent->brain.FindOutput(c.out)];
		c2.weight = c.weight;
		this->brain.input_to_output.emplace_back(c2);
	}
	
	// Mutations
	// Remove hidden
	double c_remove_hidden = Random::RandomDouble(0.0, 1.0);
	if (c_remove_hidden < sim->mutation_remove_hidden && brain.hidden.size() >= 1) {
		int index;
		Neuron* n = brain.GetRandomHiddenneuron(index);
		for (int i = 0; i < brain.input_to_hidden.size(); i++) {
			if (brain.input_to_hidden[i].out == n) {
				brain.input_to_hidden.erase(brain.input_to_hidden.begin() + i);
				i--;
			}
		}
		for (int i = 0; i < brain.hidden_to_hidden.size(); i++) {
			if (brain.hidden_to_hidden[i].in == n || brain.hidden_to_hidden[i].out == n) {
				brain.hidden_to_hidden.erase(brain.hidden_to_hidden.begin() + i);
				i--;
			}
		}
		for (int i = 0; i < brain.hidden_to_output.size(); i++) {
			if (brain.hidden_to_output[i].in == n) {
				brain.hidden_to_output.erase(brain.hidden_to_output.begin() + i);
				i--;
			}
		}
		delete n;
		brain.hidden.erase(brain.hidden.begin() + index);
		mutation_color_change += sim->mutation_color_change_hidden;
	}
	// Add hidden
	double c_add_hidden = Random::RandomDouble(0.0, 1.0);
	if (c_add_hidden < sim->mutation_add_hidden && this->brain.hidden.size() < sim->max_hidden_neurons) {
		Neuron* n = new Neuron();
		this->brain.hidden.emplace_back(n);
		mutation_color_change += sim->mutation_color_change_hidden;
	}

	// Bias
	for (int i = 0; i < SENSOR_SIZE; i++) {
		double c_change_bias = Random::RandomDouble(0.0, 1.0);
		if (c_change_bias < sim->mutation_change_bias) {
			double d = Random::RandomDouble(-sim->mutation_change_bias_rate, sim->mutation_change_bias_rate);
			brain.input[i].bias += d;
			mutation_color_change += abs(d);
		}
	}
	for (int i = 0; i < brain.hidden.size(); i++) {
		double c_change_bias = Random::RandomDouble(0.0, 1.0);
		if (c_change_bias < sim->mutation_change_bias) {
			double d = Random::RandomDouble(-sim->mutation_change_bias_rate, sim->mutation_change_bias_rate);
			brain.hidden[i]->bias += d;
			mutation_color_change += abs(d);
		}
	}
	for (int i = 0; i < ACTION_SIZE; i++) {
		double c_change_bias = Random::RandomDouble(0.0, 1.0);
		if (c_change_bias < sim->mutation_change_bias) {
			double d = Random::RandomDouble(-sim->mutation_change_bias_rate, sim->mutation_change_bias_rate);
			brain.output[i].bias += d;
			mutation_color_change += abs(d);
		}
	}

	// Weights
	for (int i = 0; i < this->brain.input_to_hidden.size(); i++) {
		double c_change_weight = Random::RandomDouble(0.0, 1.0);
		if (c_change_weight < sim->mutation_change_weight) {
			Connection* c = &this->brain.input_to_hidden[i];
			double d = Random::RandomDouble(-sim->mutation_change_weight_rate, sim->mutation_change_weight_rate);
			c->weight += d;
			mutation_color_change += abs(d);
		}
	}
	for (int i = 0; i < this->brain.hidden_to_hidden.size(); i++) {
		double c_change_weight = Random::RandomDouble(0.0, 1.0);
		if (c_change_weight < sim->mutation_change_weight) {
			Connection* c = &this->brain.hidden_to_hidden[i];
			double d = Random::RandomDouble(-sim->mutation_change_weight_rate, sim->mutation_change_weight_rate);
			c->weight += d;
			mutation_color_change += abs(d);
		}
	}
	for (int i = 0; i < this->brain.hidden_to_output.size(); i++) {
		double c_change_weight = Random::RandomDouble(0.0, 1.0);
		if (c_change_weight < sim->mutation_change_weight) {
			Connection* c = &this->brain.hidden_to_output[i];
			double d = Random::RandomDouble(-sim->mutation_change_weight_rate, sim->mutation_change_weight_rate);
			c->weight += d;
			mutation_color_change += abs(d);
		}
	}
	for (int i = 0; i < this->brain.input_to_output.size(); i++) {
		double c_change_weight = Random::RandomDouble(0.0, 1.0);
		if (c_change_weight < sim->mutation_change_weight) {
			Connection* c = &this->brain.input_to_output[i];
			double d = Random::RandomDouble(-sim->mutation_change_weight_rate, sim->mutation_change_weight_rate);
			c->weight += d;
			mutation_color_change += abs(d);
		}
	}

	// Add Connection
	double c_add_weight = Random::RandomDouble(0.0, 1.0);
	while (c_add_weight < sim->mutation_add_weight) {
		Connection c;
		mutation_color_change += sim->mutation_color_change_weight;
		if (brain.hidden.size() != 0) {
			int a = Random::RandomInt(0, 4);

			if (a == 0 && brain.hidden.size() != 0) { // Input -> Hidden
				c.in = brain.GetRandomInputneuron();
				c.out = brain.GetRandomHiddenneuron();
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
				c.in = brain.GetRandomHiddenneuron();
				c.out = brain.GetRandomHiddenneuron();
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
				c.in = brain.GetRandomHiddenneuron();
				c.out = brain.GetRandomOutputneuron();
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
				c.in = brain.GetRandomInputneuron();
				c.out = brain.GetRandomOutputneuron();
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
			c.in = brain.GetRandomInputneuron();
			c.out = brain.GetRandomOutputneuron();
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