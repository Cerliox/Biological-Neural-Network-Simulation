#include "BioSimulation.h"


int main()
{
	BioSimulation* sim = new BioSimulation("Config.ini");

	CImgDisplay disp;

	if (sim->save_video) {
		CImgList<unsigned char> images;

		for (int i = 0; i < sim->save_fps * sim->save_length; i++) {
			images.insert(sim->CreateImage());
			sim->Update();
		}

		images.save_video("out.mp4");
	}
	else {
		disp.display(sim->CreateImage());
		disp.show();
		while(!disp.is_closed()) {
			disp.display(sim->CreateImage());
			sim->Update();
			Sleep(sim->video_sleep);
		}
	}

	return 0;
}
		