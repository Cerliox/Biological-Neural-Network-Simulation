#include "BioSimulation.h"

bool CollisionDetection::Collide(Organism* o, Food* f) {
	int width = o->sim->organism_width;
	int height = o->sim->organism_height;

	return f->x >= o->x && f->x <= o->x + width && f->y >= o->y && f->y <= o->y + height;
}

BioSimulation::BioSimulation(String configfilename) {
	srand(time(nullptr));
	this->config_reader = new INIReader(configfilename);

	// Max options
	max_x = config_reader->GetInteger("Max", "x", 400);
	max_y = config_reader->GetInteger("Max", "y", 400);
	max_organisms = config_reader->GetInteger("Max", "Organisms", 1000);
	max_energy = config_reader->GetReal("Max", "Energy", 100.0);
	max_speed = config_reader->GetReal("Max", "Speed", 10.0);
	max_health = config_reader->GetReal("Max", "Health", 100.0);
	max_food = config_reader->GetInteger("Max", "Food", 1000);
	max_duplication = config_reader->GetReal("Max", "Duplication", 100.0);

	// Start options
	start_organisms = config_reader->GetInteger("Start", "Organisms", 100);
	start_health = config_reader->GetReal("Start", "Health", max_health);
	start_energy = config_reader->GetReal("Start", "Energy", max_energy);
	start_food = config_reader->GetInteger("Start", "Food", 100);

	// Organisms
	organism_width = config_reader->GetInteger("Organism", "Width", 5);
	organism_height = config_reader->GetInteger("Organism", "Height", 5);
	organism_energy_refreshrate = config_reader->GetReal("Organism", "Energyrefreshrate", 1.0);
	organism_duplication_amount = config_reader->GetInteger("Organism", "Duplicationamount", 2);
	organism_color[0] = config_reader->GetInteger("Organism", "R", 0);
	organism_color[1] = config_reader->GetInteger("Organism", "G", 255);
	organism_color[2] = config_reader->GetInteger("Organism", "B", 0);

	// Input layer
	start_ih_connections = config_reader->GetInteger("Inputlayer", "InputToHidden", 3);
	start_io_connections = config_reader->GetInteger("Inputlayer", "InputToOutput", 2);
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Inputlayer", "ActivationFunction", "Linear"), brain_input_activation_function);

	// Hidden layer
	start_hidden_neurons = config_reader->GetInteger("Hiddenlayer", "Neurons", 4);
	start_hh_connections = config_reader->GetInteger("Hiddenlayer", "HiddenToHidden", 3);
	start_ho_connections = config_reader->GetInteger("Hiddenlayer", "HiddenToOutput", 3);
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Hiddenlayer", "ActivationFunction", "Sigmoid"), brain_hidden_activation_function);

	// Output layer
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Outputlayer", "ActivationFunction", "Linear"), brain_output_activation_function);

	// Food
	food_per_iteration = config_reader->GetReal("Food", "PerIteration", 1.0);
	food_refresh = config_reader->GetReal("Food", "Refresh", 50.0);
	
	// Health
	health_loss_rate = config_reader->GetReal("Health", "Lossrate", 1.0);

	// Mutation
	mutation_add_hidden = config_reader->GetReal("Mutation", "Addhidden", 0.15);
	mutation_add_weight = config_reader->GetReal("Mutation", "Addweight", 0.3);
	mutation_weight = config_reader->GetReal("Mutation", "Weight", 0.1);
	mutation_bias = config_reader->GetReal("Mutation", "Bias", 0.1);

	// Save
	save_video = config_reader->GetBoolean("Save", "Video", true);
	save_length = config_reader->GetInteger("Save", "Length", 10);
	save_fps = config_reader->GetInteger("Save", "FPS", 25);

	// Video
	video_sleep = config_reader->GetInteger("Video", "Sleep", 50);

	// Spawn organsisms
	int o = std::min(max_organisms, start_organisms);

	organisms.reserve(max_organisms);
	Organism* s_organisms = new Organism[o];
	for (int i = 0; i < o; i++) {
		s_organisms[i].Init(this);
		organisms.push_back(&s_organisms[i]);
	}
	// Spawn food
	o = std::min(max_food, start_food);
	food.reserve(max_food);
	Food* s_food = new Food[o];
	for (int i = 0; i < o; i++) {
		s_food[i].x = Random::RandomInt(0, max_x);
		s_food[i].y = Random::RandomInt(0, max_y);
		s_food[i].sim = this;
		food.push_back(&s_food[i]);
	}
	
}

Image BioSimulation::CreateImage() {
	Image img(max_x, max_y, 1, 3, 0);

	for (int i = 0; i < food.size(); i++) {
		Food* f = food[i];
		img(f->x, f->y, 0) = 255;
	}
	for (int i = 0; i < organisms.size(); i++) {
		Organism* o = organisms[i];
		img.draw_rectangle(o->x, o->y, o->x + organism_width, o->y + organism_height, organism_color);
	}

	return img;
}

void BioSimulation::Update() {
	for (int i = 0; i < organisms.size(); i++) {
		organisms[i]->Update();
		if (organisms[i]->health <= 0.0) {
			organisms.erase(organisms.begin() + i);
			i--;
		}
		else if (organisms[i]->duplication >= max_duplication) {
			if (organisms.size() < max_organisms) {
				organisms[i]->duplication = 0.0;
				int am = fmin(max_organisms - organisms.size(), organism_duplication_amount);
				for (int a = 0; a < am; a++) {
					Organism* o = new Organism();
					o->Inherit(organisms[i]);
					organisms.emplace_back(o);
				}
			}
		}
	}

	if (food.size() <= max_food) {
		foodcounter += food_per_iteration;
		int spawn = (int)foodcounter;
		foodcounter -= (double) spawn;

		if (food.size() + spawn > max_food)
			spawn = max_food - food.size();

		for (int i = 0; i < spawn; i++) {
			Food* f = new Food();
			f->x = Random::RandomInt(0, max_x);
			f->y = Random::RandomInt(0, max_y);
			f->sim = this;
			food.emplace_back(f);
		}
	}
	else 
		foodcounter = 0.0;
	
}