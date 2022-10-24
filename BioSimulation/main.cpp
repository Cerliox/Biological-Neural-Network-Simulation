#include "BioSimulation.h"


int main()
{
	CImgList<unsigned char> images;
	BioSimulation* sim = new BioSimulation("Config.ini");

	for (int i = 0; i < 25 * 10; i++) {
		images.insert(sim->CreateImage());
		sim->Update();
	}

	images.save_video("out.mp4");

	return 0;
}
		