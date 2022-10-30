# :earth_americas:Biologic Neural Network Simulation:earth_americas:
This is a project made for fun so don't expect well-documented code.
## What is this?
This simulation consists of small creatures called Organisms and food. The Organisms are updated every frame independant from any other Organism.
Their behaviour comes from a neural network running in the background. It consists of the inputs:

- ``Health`` (Current health)
- ``Energy`` (Current amount of energy)
- ``x`` (X coordinate ranging from -0.5 to 0.5
- ``y`` (Y coordinate ranging from -0.5 to 0.5
- ``Dx`` (X distance to closest food)
- ``Dy`` (Y distance to closest food)

and the outputs:

- X (X direction of the next move)
- Y (Y direction of the next move)
- Speed (Speed of the next move)

They loose health every iteration and need to eat food to heal themselves. When they have eaten enough, they replicate themselves.
The replicated Organism has some mutation inside his brain structure causing a different behaviour. This leads to a learing algorithm for those
neural network to eat as much food as possible. When they run out of energy they loose more health.
The brain complexity is computed every iteration via summing up every weight and amount of neurons so that an Organism consumes more energy when he has more neuron

![image](https://user-images.githubusercontent.com/52610941/198897277-a0bd28a1-23ed-44b9-9638-d22aa4ff13e0.png)

This simulation can output detailed statistics of each iteration as well as a video of the whole simulation and every brain on the last iteration.
It can even be displayed at lifetime.

The python-script ``analyse.py`` can be run to visualize the statistics and generate graphs of the neural network of the last brains. Be aware that the path of the brains file inside this script is hardcoded as well as some option. Change those to achieve your results

```
statistics_path = "out/build/x64-debug/BioSimulation/data/statistics.txt"
brains_path = "out/build/x64-debug/BioSimulation/data/brains.txt"

show_statistics = 0
show_brains = 1

label_weights = 1
label_biases = 0
```

![image](https://user-images.githubusercontent.com/52610941/198900402-1afd87ed-51ad-493e-be61-709fb28df822.png)

## Dependencies

This simulation is very lightweight and only makes use of 2 libraries. They are already included inside the header files so no extra files are needed:

- CImg (Graphics library: dependencies/Cimg/Cimg.h)
- inih (Library for reading .ini files: dependencies/inih/INIReader.h)

Their versions can be found inside the folder inside the file "version.txt"

The python-script uses:

``matplotlib >= 3.2.1``

``numpy >= 1.23.3``

``scipy >= 1.4.1``

``graphviz >= 0.20.1``

``colour >= 0.1.5``

Every python dependency can be installed via ``pip install``


## Installation

This program was made via CMake so you can compile it yourself. 
Just run this command:

``cmake .``

(https://cmake.org/)

## Settings
You can change a lot inside the ``Config.ini`` file. In this example the default settings are used

```
[Max]
x=1000 (Screen width and max x position: int)
y=800 (Screen height and max y position: int)
Organisms=500 (Max amount of organisms: int)
Energy=100.0 (Max amount of energy: double)
Speed=2.0 (They can't go faster: double)
Health=100.0 (Max amount of health: double)
Food=2500 (Max amount of spawned food inside the world: int)
Replication=100.0 (Amount of replication an organism has to achieve to replicate: double)

[Start]
Organisms=300 (Starting amount of Organisms: int)
Health=100.0 (Their start health: double)
Energy=100.0 (Their start energy: double)
Food=300 (Amount of spawned food at the beginning: int)

[Organism]
Width=5 (int)
Height=5 (int)
Energyrefreshrate=10.0 (Energy they receive every round: double)
Replicationamount=1 (Amount of children spawning every replication: int)
Losscomplexitymultiplier=0.1 (Mutiply with the loss of energy by the complexity: double)
Lossspeedmultiplier=0.01 (Multiply with the loss of energy by the speed: double)
Failsafe=true (The last Organism does not die and gets teleported around the world to achieve no extinction: bool)
Healthloss=1.0 (Amount of health they loose every iteration: double)

[Inputlayer]
InputToHidden=0 (Amount of connections to the hidden layer at the beginning: int)
InputToOutput=5 (Amount of connections to the output layer at the beginning: int)
ActivationFunction=Linear (The activationfunction of the neuron: See Activationfunctions: String)

[Hiddenlayer]
Neurons=5 (Start amount of hidden neurons: int)
HiddenToHidden=0 (Start amount of connections to any neuron in the hidden layer: int)
HiddenToOutput=0 (Start amount of connections to the output layer: int)
ActivationFunction=Sigmoid (String)

[Outputlayer]
ActivationFunction=Linear (String)

[Food]
PerIteration=1.0 (Amount of food to spawn every iteration: double)
Refresh=50.0 (Amount of health, energy and replication every food refreshes: double)
R=255 (Color R: char)
G=0 (Color G: char)
B=0 (Color B: char)

[Mutation]
Addhidden=0.0 (Chance of adding a hidden neuron: double)
Removehidden=0.0 (Chance of removing a hidden neuron: double)
Addweight=0.3 (Chance of adding a weight: double)
Changeweight=0.4 (Chance of changing a weight: double)
Weightrate=0.5 (When changing a weight it adds a random number between this range to it: double)
Changebias=0.3 (Chance of changing a bias: double)
Biasrate=0.5 (When changing a bias it adds a random number between this range to it: double)
Maxcolorchange=40.0 (The max amount the color is able to change when mutating: double)
Colorchangemultiplier=10.0 (Multiplied with the amount of mutations: double)
Coloraddhidden=5.0 (Amount of color changing when adding a hidden neuron: double)
Coloraddweight=2.0 (Amount of color changing when adding a weight: double)

[Display]
Show=true (Shows a realtime display of the simulation: bool)
Sleep=0 (To sleep between every iteration: int)

Statistics=true (Show realtime statistics of the simulation: bool)
Bufferlength=100 (Buffer for the statistics: int)
Statisticsx=600 (Width of the statistics window: int)
Statisticsmaxy=800 (The graph of the statistics ranges from 0 to this value: int)

[Save]
Folder=data (Folder to save data to: String)
Video=false (Save video file: bool)
Videofilename=out.mp4 (Name of the videoname: String)
Length=10 (Length in seconds to save: int)
FPS=25 (FPS of the video: int)

Brain=true (Save every brain after the simulation in a file: bool)
Brainfilename=brains.txt (Filename where the brains are saved: String)

Statistics=false (Save statistics in a file: bool)
Extendedstatistics=false (Save extended statistics in a file: bool)
Statisticsfilename=statistics.txt (Filename where all the statistics are stored: String)

[Autoreset]
Active=true (When there is only one Organism reset after a certain amount of iteration: bool)
Iterations=150 (Amount of iterations with only one Organism needed to reset the simulation: int)
```

### Activationfunctions
Sigmoid https://en.wikipedia.org/wiki/Sigmoid_function

ReLU https://en.wikipedia.org/wiki/Rectifier_(neural_networks)

Linear
