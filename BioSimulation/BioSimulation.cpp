#include "BioSimulation.h"

BioSimulation::BioSimulation(String configfilename) {
	srand(time(nullptr));
	this->config_reader = new INIReader(configfilename);

	max_x = config_reader->GetInteger("Max", "x", 400);
	max_y = config_reader->GetInteger("Max", "y", 400);
	max_organisms = config_reader->GetInteger("Max", "Organisms", 100);
	max_energy = config_reader->GetReal("Max", "Energy", 100.0);
	max_speed = config_reader->GetReal("Max", "Speed", 10.0);

	start_organisms = config_reader->GetInteger("Start", "Organisms", 100);
	start_health = config_reader->GetReal("Start", "Health", 100.0);
	start_energy = config_reader->GetReal("Start", "Energy", 100.0);
	start_hidden_neurons = config_reader->GetInteger("Start", "Hiddenneurons", 4);
	start_ih_connections = config_reader->GetInteger("Start", "InputToHidden", 3);
	start_hh_connections = config_reader->GetInteger("Start", "HiddenToHidden", 3);
	start_ho_connections = config_reader->GetInteger("Start", "HiddenToOutput", 3);
	start_io_connections = config_reader->GetInteger("Start", "InputToOutput", 2);

	organism_width = config_reader->GetInteger("Organism", "Width", 5);
	organism_height = config_reader->GetInteger("Organism", "Height", 5);
	organism_energy_refreshrate = config_reader->GetReal("Organism", "Energyrefreshrate", 1.0);
	organism_color[0] = config_reader->GetInteger("Organism", "R", 0);
	organism_color[1] = config_reader->GetInteger("Organism", "G", 255);
	organism_color[2] = config_reader->GetInteger("Organism", "B", 0);

	threshold_speed = config_reader->GetReal("Threshold", "Speed", 0.1);

	int o;
	if (max_organisms < start_organisms)
		o = max_organisms;
	else
		o = start_organisms;

	organisms.reserve(o);
	Organism* h_organisms = new Organism[o];
	for (int i = 0; i < o; i++) {
		h_organisms[i].Init(this);
		organisms.push_back(&h_organisms[i]);
	}
	
}

Image BioSimulation::CreateImage() {
	Image img(max_x, max_y, 1, 3, 0);

	for (int i = 0; i < organisms.size(); i++) {
		Organism* o = organisms[i];
		img.draw_rectangle(o->x, o->y, o->x + organism_width, o->y + organism_height, organism_color);
	}

	return img;
}

void BioSimulation::Update() {
	for (int i = 0; i < organisms.size(); i++) {
		organisms[i]->Update();
	}
}