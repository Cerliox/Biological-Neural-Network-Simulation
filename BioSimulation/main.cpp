#include "BioSimulation.h"


int main()
{
	BioSimulation* sim = new BioSimulation("Config.ini");

	CImgDisplay disp;
	CImgList<unsigned char> images;
	Image curr;
	std::ofstream file_statistics;
	if (sim->save_statistics) {
		file_statistics.open(sim->save_statistics_filename, std::ios::out | std::ios::trunc);
		file_statistics << sim->save_extended_statistics << std::endl;
	}

	for (int iteration = 0;; iteration++) {
		curr = sim->CreateImage();

		if (sim->save_statistics) {
			sim->WriteStatistics(file_statistics);
		}
		if (sim->save_video) {
			images.insert(curr);
			if (iteration >= sim->save_fps * sim->save_length)
				break;
		}
		if (sim->display_simulation) {
			disp.display(curr);
			if (disp.is_closed())
				break;
			Sleep(sim->display_sleep);
		}

		sim->Update();
	}

	if (sim->save_statistics) {
		std::cout << "Statistics written to " << sim->save_statistics_filename << std::endl;
		file_statistics.close();
	}
	

	if (sim->save_video) {
		images.save_video(sim->save_video_filename.c_str());
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
		