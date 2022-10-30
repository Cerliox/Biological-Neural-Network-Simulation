#include "BioSimulation.h"
#include "Vector.h"

#include <intrin.h>

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

Neuron* Brain::GetRandomInputneuron() {
	return &input[Random::RandomInt(0, SENSOR_SIZE)];
}
Neuron* Brain::GetRandomHiddenneuron() {
	return hidden[Random::RandomInt(0, hidden.size())];
}
Neuron* Brain::GetRandomHiddenneuron(int& index) {
	index = Random::RandomInt(0, hidden.size());
	return hidden[index];
}
Neuron* Brain::GetRandomOutputneuron() {
	return &output[Random::RandomInt(0, ACTION_SIZE)];
}

int Brain::FindInput(Neuron* n) {
	return n->sensor;
}
int Brain::FindHidden(Neuron* n) {
	for (int i = 0; i < hidden.size(); i++)
		if (hidden[i] == n)
			return i;
	__debugbreak();
}
int Brain::FindOutput(Neuron* n) {
	return n->action;
}

void Neuron::Clear() {
	this->value = 0.0;
}

bool Connection::operator==(Connection& o) {
	return o.in == this->in && o.out == this->out;
}

void Brain::Behave() {
	for (int i = 0; i < SENSOR_SIZE; i++) {
		if (input[i].sensor == HEALTH)
			input[i].value = organism->health / organism->sim->max_health;
		else if (input[i].sensor == ENERGY) {
			input[i].value = organism->energy;
		}
		else if (input[i].sensor == PX) {
			input[i].value = ((double)organism->x / (double)organism->sim->max_x) - 0.5;
		}
		else if (input[i].sensor == PY) {
			input[i].value = ((double)organism->y / (double)organism->sim->max_y) - 0.5;
		}
		else if (input[i].sensor == DFX) {
			if (organism->closest_food != nullptr)
				input[i].value = organism->distance_to_closest_food.x;
		}
		else if (input[i].sensor == DFY) {
			if (organism->closest_food != nullptr)
				input[i].value = organism->distance_to_closest_food.y;
		}
	}
	
	// Clear values
	for (int i = 0; i < hidden.size(); i++) {
		hidden[i]->Clear();
		hidden[i]->value += hidden[i]->bias;
	}
	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].Clear();
		output[i].value += output[i].bias;
	}

	// Compute hidden
	for (Connection c : input_to_hidden) {
		c.out->value += c.in->value * c.weight; // No out-of-bounds checks
	}
	for (Connection c : hidden_to_hidden) {
		c.out->value += c.in->value * c.weight; // No out-of-bounds checks
	}

	for (int i = 0; i < hidden.size(); i++) {
		hidden[i]->value = this->organism->sim->brain_hidden_activation_function(hidden[i]->value);
	}

	// Compute output
	for (Connection c : hidden_to_output) {
		c.out->value += c.in->value * c.weight; // No out-of-bounds checks
	}
	for (Connection c : input_to_output) {
		c.out->value += c.in->value * c.weight; // No out-of-bounds checks
	}

	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].value = this->organism->sim->brain_output_activation_function(output[i].value);
	}

	// Translate output
	for (int i = 0; i < ACTION_SIZE; i++) {
		if (output[i].action == MOVEX) {
			organism->direction.x += output[i].value;
		}
		else if (output[i].action == MOVEY) {
			organism->direction.y += output[i].value;
		}
		else if (output[i].action == MOVESPEED) {
			organism->speed += output[i].value;
		}
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
		file << std::to_string(hidden[i]->bias) << " ";
	}
	file << std::endl;
	for (int i = 0; i < ACTION_SIZE; i++) {
		file << std::to_string(output[i].bias) << " ";
	}
	file << std::endl;
	for (Connection c : input_to_hidden) {
		file << std::to_string(FindInput(c.in)) << " " << std::to_string(FindHidden(c.out)) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : hidden_to_hidden) {
		file << std::to_string(FindHidden(c.in)) << " " << std::to_string(FindHidden(c.out)) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : hidden_to_output) {
		file << std::to_string(FindHidden(c.in)) << " " << std::to_string(FindOutput(c.out)) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
	for (Connection c : input_to_output) {
		file << std::to_string(FindInput(c.in)) << " " << std::to_string(FindOutput(c.out)) << " " << std::to_string(c.weight) << ";";
	}
	file << std::endl;
}