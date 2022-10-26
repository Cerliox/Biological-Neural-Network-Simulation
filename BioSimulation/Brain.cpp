#include "BioSimulation.h"
#include "Vector.h"

double ActivationFunctions::ReLU(double in) {
	return fmin(0.0, in);
}
double ActivationFunctions::Linear(double in) {
	return in;
}
double ActivationFunctions::Sigmoid(double in) {
	return 1.0 / (1.0 + pow(EULER, -in));
}
void ActivationFunctions::GetActivationFunction(String name, double(*&func)(double)) {
	if (name == "ReLU") {
		func = ActivationFunctions::ReLU;
	}
	else if (name == "Linear") {
		func = ActivationFunctions::Linear;
	}
	else if (name == "Sigmoid") {
		func = ActivationFunctions::Sigmoid;
	}
	else {
		func = ActivationFunctions::ReLU;
	}
}

Brain::Brain() {
}

void Inputneuron::GetValue(Brain* brain) {
	this->value = 0.0f;
	if (this->sensor == HEALTH) {
		this->value = brain->organism->health / brain->organism->sim->max_health;
	}
	else if (this->sensor == ENERGY) {
		this->value = brain->organism->energy;
	}
	else if (this->sensor == PX) {
		this->value = ((double) brain->organism->x / (double)brain->organism->sim->max_x) - 0.5;
	}
	else if (this->sensor == PY) {
		this->value = ((double )brain->organism->y / (double)brain->organism->sim->max_y) - 0.5;
	}
	else if (this->sensor == DFX) {
		if (brain->organism->closest_food != nullptr)
			this->value = brain->organism->distance_to_closest_food.x;
	}
	else if (this->sensor == DFY) {
		if (brain->organism->closest_food != nullptr)
			this->value = brain->organism->distance_to_closest_food.y;
	}
	/*else if (this->sensor == DCEX) {
		if(brain->organism->closest != nullptr)
			this->value = brain->organism->distance_to_closest.x;
	}
	else if (this->sensor == DCEY) {
		if (brain->organism->closest != nullptr)
			this->value = brain->organism->distance_to_closest.y;
	}*/

	this->value += bias;
	this->value = brain->organism->sim->brain_input_activation_function(this->value);
}
void Hiddenneuron::Clear() {
	this->value = 0.0;
}
void Outputneuron::Clear() {
	this->value = 0.0;
}
void Outputneuron::TranslateValue(Organism* organism) {
	if (this->action == MOVEX) {
		organism->direction.x += this->value;
	}
	else if (this->action == MOVEY) {
		organism->direction.y += this->value;
	}
	else if (this->action == MOVESPEED) {
		organism->speed += this->value;
	}
}

bool Connection::operator==(Connection& o) {
	return o.in == this->in && o.out == this->out;
}

void Brain::Behave() {
	for (int i = 0; i < SENSOR_SIZE; i++) {
		input[i].GetValue(this);
	}
	
	// Clear values
	for (int i = 0; i < hidden.size(); i++) {
		hidden[i].Clear();
		hidden[i].value += hidden[i].bias;
	}
	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].Clear();
		output[i].value += output[i].bias;
	}

	// Compute hidden
	for (Connection c : input_to_hidden) {
		hidden[c.out].value += input[c.in].value * c.weight; // No out-of-bounds checks
	}
	for (Connection c : hidden_to_hidden) {
		hidden[c.out].value += hidden[c.in].value * c.weight;
	}

	for (int i = 0; i < hidden.size(); i++) {
		hidden[i].value = this->organism->sim->brain_hidden_activation_function(hidden[i].value);
	}

	// Compute output
	for (Connection c : hidden_to_output) {
		output[c.out].value += hidden[c.in].value * c.weight;
	}
	for (Connection c : input_to_output) {
		output[c.out].value += input[c.in].value * c.weight;
	}

	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].value = this->organism->sim->brain_output_activation_function(output[i].value);
	}

	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].TranslateValue(this->organism);
	}
}
void Brain::ComputeComplexity() {
	complexity = 0.0;
	complexity += this->hidden.size();
	complexity += this->input_to_hidden.size();
	complexity += this->hidden_to_hidden.size();
	complexity += this->hidden_to_output.size();
	complexity += this->input_to_output.size();
}
void Brain::SaveToFile(std::ofstream& file) {
	file << std::string(organism->sim->config_reader->GetString("Inputlayer", "ActivationFunction", "Linear")) << " " << std::string(organism->sim->config_reader->GetString("Hiddenlayer", "ActivationFunction", "Sigmoid")) << " " << std::string(organism->sim->config_reader->GetString("Outputlayer", "ActivationFunction", "Linear")) << std::endl;
	for (int i = 0; i < SENSOR_SIZE; i++) {
		file << std::to_string(input[i].bias) << " ";
	}
	file << std::endl;
	for (int i = 0; i < hidden.size(); i++) {
		file << std::to_string(hidden[i].bias) << " ";
	}
	file << std::endl;
	for (int i = 0; i < ACTION_SIZE; i++) {
		file << std::to_string(output[i].bias) << " ";
	}
	file << std::endl;
	for (Connection c : input_to_hidden) {
		file << std::to_string(c.in) << " " << std::to_string(c.out) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : hidden_to_hidden) {
		file << std::to_string(c.in) << " " << std::to_string(c.out) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : hidden_to_output) {
		file << std::to_string(c.in) << " " << std::to_string(c.out) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : input_to_output) {
		file << std::to_string(c.in) << " " << std::to_string(c.out) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
}