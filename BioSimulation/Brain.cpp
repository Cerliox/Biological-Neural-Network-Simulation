#include "BioSimulation.h"
#include "Vector.h"

Brain::Brain() {
}

void Inputneuron::GetValue(Brain* brain) {
	this->value = 0.0f;
	if (this->sensor == HEALTH) {
		this->value = brain->organism->health;
	}
	else if (this->sensor == ENERGY) {
		this->value = brain->organism->energy;
	}

	this->value += bias;
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
		if (this->value > organism->sim->threshold_speed)
			organism->speed = this->value;
		else
			organism->speed = 0.0;
	}
}

bool Connection::operator==(Connection& o) {
	return o.in == this->in && o.out == this->out;
}

void Brain::Behave() {
	for (int i = 0; i < SENSOR_SIZE; i++) {
		input[i].GetValue(this);
	}
	
	for (int i = 0; i < hidden.size(); i++) {
		hidden[i].Clear();
		hidden[i].value += hidden[i].bias;
	}
	for (Connection c : input_to_hidden) {
		hidden[c.out].value += input[c.in].value * c.weight; // No out-of-bounds checks
	}
	for (Connection c : hidden_to_hidden) {
		hidden[c.out].value += hidden[c.in].value * c.weight;
	}

	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].Clear();
		output[i].value += output[i].bias;
	}
	for (Connection c : hidden_to_output) {
		output[c.out].value += hidden[c.in].value * c.weight;
	}
	for (Connection c : input_to_output) {
		output[c.out].value += input[c.in].value * c.weight;
	}

	for (int i = 0; i < ACTION_SIZE; i++) {
		output[i].TranslateValue(this->organism);
	}
}