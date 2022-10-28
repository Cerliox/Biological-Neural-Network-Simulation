#include "BioSimulation.h"

const unsigned char blue[] = { 0, 0, 255 };
const unsigned char red[] = { 255, 0, 0 };
const unsigned char black[] = { 0,0,0 };
const unsigned char white[] = { 255, 255, 255 };

bool CollisionDetection::Collide(Organism* o, Food* f) {
	int width = o->sim->organism_width;
	int height = o->sim->organism_height;

	return f->x >= o->x && f->x <= o->x + width && f->y >= o->y && f->y <= o->y + height;
}

BioSimulation::BioSimulation(String cfn) {
	this->configfilename = cfn;
	srand(time(nullptr));
	LoadConfig();
	Spawn();

	// Set display buffer if needed
	if (display_statistics) {
		display_statistics_organism_data.reserve(display_statistics_data_length);
		display_statistics_food_data.reserve(display_statistics_data_length);
	}
	
}

void BioSimulation::LoadConfig() {
	this->config_reader = new INIReader(configfilename);

	// Max options
	max_x = config_reader->GetInteger("Max", "x", 1000);
	max_y = config_reader->GetInteger("Max", "y", 800);
	max_organisms = config_reader->GetInteger("Max", "Organisms", 500);
	max_energy = config_reader->GetReal("Max", "Energy", 100.0);
	max_speed = config_reader->GetReal("Max", "Speed", 7.0);
	max_health = config_reader->GetReal("Max", "Health", 100.0);
	max_food = config_reader->GetInteger("Max", "Food", 2500);
	max_replication = config_reader->GetReal("Max", "Replication", 50.0);

	// Start options
	start_organisms = config_reader->GetInteger("Start", "Organisms", 300);
	start_health = config_reader->GetReal("Start", "Health", max_health);
	start_energy = config_reader->GetReal("Start", "Energy", max_energy);
	start_food = config_reader->GetInteger("Start", "Food", 300);

	// Organisms
	organism_width = config_reader->GetInteger("Organism", "Width", 5);
	organism_height = config_reader->GetInteger("Organism", "Height", 5);
	organism_energy_refreshrate = config_reader->GetReal("Organism", "Energyrefreshrate", 10.0);
	organism_duplication_amount = config_reader->GetInteger("Organism", "Replicationamount", 3);
	organism_energy_loss_complexity_multiplier = config_reader->GetReal("Organism", "Losscomplexitymultiplier", 0.1);
	organism_energy_loss_speed_multiplier = config_reader->GetReal("Organism", "Lossspeedmultiplier", 0.01);
	organism_failsafe = config_reader->GetBoolean("Organism", "Failsafe", true);
	health_loss_rate = config_reader->GetReal("Organism", "Healthloss", 3.0);

	// Input layer
	start_ih_connections = config_reader->GetInteger("Inputlayer", "InputToHidden", 0);
	start_io_connections = config_reader->GetInteger("Inputlayer", "InputToOutput", 5);
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Inputlayer", "ActivationFunction", "Linear"), brain_input_activation_function);

	// Hidden layer
	start_hidden_neurons = config_reader->GetInteger("Hiddenlayer", "Neurons", 0);
	start_hh_connections = config_reader->GetInteger("Hiddenlayer", "HiddenToHidden", 0);
	start_ho_connections = config_reader->GetInteger("Hiddenlayer", "HiddenToOutput", 0);
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Hiddenlayer", "ActivationFunction", "Sigmoid"), brain_hidden_activation_function);

	// Output layer
	ActivationFunctions::GetActivationFunction(config_reader->GetString("Outputlayer", "ActivationFunction", "Linear"), brain_output_activation_function);

	// Food
	food_per_iteration = config_reader->GetReal("Food", "PerIteration", 2.5);
	food_refresh = config_reader->GetReal("Food", "Refresh", 50.0);
	food_color[0] = config_reader->GetInteger("Food", "R", 255);
	food_color[1] = config_reader->GetInteger("Food", "G", 0);
	food_color[2] = config_reader->GetInteger("Food", "B", 0);

	// Mutation
	mutation_add_hidden = config_reader->GetReal("Mutation", "Addhidden", 0.1);
	mutation_remove_hidden = config_reader->GetReal("Mutation", "Removehidden", 0.05);
	mutation_add_weight = config_reader->GetReal("Mutation", "Addweight", 0.1);
	mutation_weight = config_reader->GetReal("Mutation", "Weight", 0.2);
	mutation_bias = config_reader->GetReal("Mutation", "Bias", 0.2);
	mutation_color_max_change = config_reader->GetReal("Mutation", "Maxcolorchange", 40.0);
	mutation_color_change_multiplier = config_reader->GetReal("Mutation", "Colorchangemultiplier", 10.0);
	mutation_color_change_hidden = config_reader->GetReal("Mutation", "Coloraddhidden", 5.0);
	mutation_color_change_weight = config_reader->GetReal("Mutation", "Coloraddweight", 2.0);

	// Display
	display_simulation = config_reader->GetBoolean("Display", "Show", true);
	display_sleep = config_reader->GetInteger("Display", "Sleep", 0);

	display_statistics = config_reader->GetBoolean("Display", "Statistics", true);
	display_statistics_data_length = config_reader->GetInteger("Display", "Bufferlength", 100);
	display_statistics_size_x = config_reader->GetInteger("Display", "Statisticsx", 600);
	display_statistics_max_y = config_reader->GetInteger("Display", "Statisticsmaxy", 800);

	// Save
	save_video = config_reader->GetBoolean("Save", "Video", false);
	save_video_filename = config_reader->GetString("Save", "Videofilename", "data/out.mp4");
	save_length = config_reader->GetInteger("Save", "Length", 10);
	save_fps = config_reader->GetInteger("Save", "FPS", 25);

	save_last_brains = config_reader->GetBoolean("Save", "Brain", true);
	save_last_brains_filename = config_reader->GetString("Save", "Brainfilename", "data/brains.txt");

	save_statistics = config_reader->GetBoolean("Save", "Statistics", false);
	save_extended_statistics = config_reader->GetBoolean("Save", "Extendedstatistics", false);
	save_statistics_filename = config_reader->GetString("Save", "Statisticsfilename", "data/statistics.txt");

	// Auto reset
	auto_reset = config_reader->GetBoolean("Autoreset", "Active", true);
	auto_reset_only_one_max = config_reader->GetInteger("Autoreset", "Ticks", 150);
}

void BioSimulation::Spawn() {
	// Spawn organsisms
	int o = std::min(max_organisms, start_organisms);

	organisms.clear();
	organisms.reserve(max_organisms);
	for (int i = 0; i < o; i++) {
		Organism* o = new Organism();
		o->Init(this);
		organisms.emplace_back(o);
	}
	// Spawn food
	o = std::min(max_food, start_food);
	food.clear();
	food.reserve(max_food);
	for (int i = 0; i < o; i++) {
		Food* f = new Food();
		f->x = Random::RandomInt(0, max_x);
		f->y = Random::RandomInt(0, max_y);
		f->sim = this;
		food.emplace_back(f);
	}
}

void BioSimulation::Reset() {
	srand(time(nullptr));
	curr_iteration = 0;
	curr_reset++;
	LoadConfig();
	Spawn();

	if (display_statistics) {
		display_statistics_organism_data.clear();
		display_statistics_food_data.clear();
	}
}

Image BioSimulation::CreateImage() {
	Image img(max_x, max_y, 1, 3, 0);

	for (int i = 0; i < food.size(); i++) {
		Food* f = food[i];
		img(f->x, f->y, 0) = food_color[0];
		img(f->x, f->y, 1) = food_color[1];
		img(f->x, f->y, 2) = food_color[2];

	}
	for (int i = 0; i < organisms.size(); i++) {
		Organism* o = organisms[i];
		img.draw_rectangle(o->x, o->y, o->x + organism_width, o->y + organism_height, o->color);
	}

	if (this->display_statistics) {
		Image stat_img(display_statistics_size_x, max_y, 1, 3);
		stat_img.fill(255);

		CImg<int> data(display_statistics_organism_data.data(), display_statistics_organism_data.size(), 1, 1, 1);
		float step_y = (float)display_statistics_max_y / max_y;
		stat_img.draw_graph(data, blue, 1.0, 2, 1, display_statistics_max_y, 0);

		data = CImg<int>(display_statistics_food_data.data(), display_statistics_food_data.size(), 1, 1, 1);
		stat_img.draw_graph(data, red, 1.0, 2, 1, display_statistics_max_y, 0);

		for (int i = 0; i < display_statistics_max_y; i += 100) {
			stat_img.draw_line(0, max_y - i / step_y, display_statistics_size_x, max_y - i / step_y, black, 1.0);
			stat_img.draw_text(0, max_y - i / step_y - 15, std::to_string(i).c_str(), black);
		}
		stat_img.draw_text(display_statistics_size_x - 120, 5, "Organisms", blue);
		stat_img.draw_text(display_statistics_size_x - 90, 18, "Food", red);
		stat_img.draw_text(5, 5, ("Iteration: " + std::to_string(curr_iteration)).c_str(), black);
		stat_img.draw_text(5, 18, ("Reset: " + std::to_string(curr_reset)).c_str(), black);
		stat_img.draw_text(5, 31, ("Time: " + std::to_string(iteration_time_ms) + "ms").c_str(), black);

		Image output;
		output.append(img, 'x');
		output.append(stat_img, 'x');

		return output;
	}

	return img;
}
void BioSimulation::WriteStatistics(std::ofstream& file) {
	file << this->organisms.size() << " " << this->food.size() << std::endl;
	if (save_extended_statistics) {
		for (Organism* o : this->organisms) {
			file << o->x << " " << o->y << " " << o->speed << " " << o->health << " " << o->energy << " " << o->replication << " " << o->amount_of_replications << " " << std::endl;
		}
	}
}

void BioSimulation::Update() {
	curr_iteration++;
	for (int i = 0; i < organisms.size(); i++) {
		organisms[i]->Update();
		if (organisms[i]->health <= 0.0) {
			if (organism_failsafe && organisms.size() == 1) {
				organisms[i]->health = 1.0;
				organisms[i]->energy += 50.0;
				organisms[i]->x = Random::RandomInt(0, max_x);
				organisms[i]->y = Random::RandomInt(0, max_y);
				continue;
			}
			Organism* o = organisms[i];
			organisms.erase(organisms.begin() + i);
			delete o;
			i--;
		}
		else if (organisms[i]->replication >= max_replication) {
			if (organisms.size() < max_organisms) {
				organisms[i]->replication = 0.0;
				int am = fmin(max_organisms - organisms.size(), organism_duplication_amount);
				organisms[i]->amount_of_replications += am;
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

	if (auto_reset) {
		if (organisms.size() <= 1) {
			auto_reset_only_one_counter++;
			if (auto_reset_only_one_counter >= auto_reset_only_one_max)
				Reset();
		}
		else {
			auto_reset_only_one_counter = 0;
		}
	}

	if (display_statistics) {
		if (display_statistics_organism_data.size() >= display_statistics_data_length)
			display_statistics_organism_data.erase(display_statistics_organism_data.begin());
		display_statistics_organism_data.emplace_back(organisms.size());
		
		if(display_statistics_food_data.size() >= display_statistics_data_length)
			display_statistics_food_data.erase(display_statistics_food_data.begin());
		display_statistics_food_data.emplace_back(food.size());
	}
}