#include "BioSimulation.h"


int main()
{
	BioSimulation* sim = new BioSimulation("Config.ini");

	CImgDisplay disp;
	disp.set_title("BioSimulation");
	CImgList<unsigned char> images;
	Image curr;
	std::ofstream file_statistics;
	if (sim->save_statistics) {
		mkdir(sim->save_folder.c_str());
		file_statistics.open(sim->save_statistics_filename, std::ios::out | std::ios::trunc);
		file_statistics << sim->save_extended_statistics << std::endl;
	}

	bool just_reseted = false;
	Clock c;
	for (int iteration = 0;; iteration++) {
		c.Start();
		sim->Update();

		if (sim->save_statistics) {
			sim->WriteStatistics(file_statistics);
		}
		if (sim->save_video) {
			images.insert(curr);
			if (iteration >= sim->save_fps * sim->save_length)
				break;
		}
		sim->iteration_time_ms = c.ElapsedMilliseconds();


		curr = sim->CreateImage();
		if (sim->display_simulation) {
			disp.display(curr);
			if (disp.is_keySPACE()) {
				if (!just_reseted) {
					sim->Reset();
					just_reseted = true;
				}
			}
			else
				just_reseted = false;
			if (disp.is_closed())
				break;
			Sleep(sim->display_sleep);
		}
	}

	if (sim->save_statistics) {
		std::cout << "Statistics written to " << sim->save_statistics_filename << std::endl;
		file_statistics.close();
	}
	
	mkdir(sim->save_folder.c_str());
	if (sim->save_video) {
		images.save_video((sim->save_folder + sim->save_video_filename).c_str());
		std::cout << "Video written to " << sim->save_video_filename << std::endl;
	}

	if (sim->save_last_brains) {
		std::ofstream file;
		file.open(sim->save_last_brains_filename, std::ios::out | std::ios::trunc);
		for (Organism* o : sim->organisms) {
			o->brain.SaveToFile(file);
		}
		std::cout << "Last brains written to " << sim->save_last_brains_filename << std::endl;
		file.close();
	}

	return 0;
}
		