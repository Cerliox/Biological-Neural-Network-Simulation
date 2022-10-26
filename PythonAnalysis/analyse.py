import matplotlib.pyplot as plt
import numpy as np
from io import SEEK_END, SEEK_SET
from scipy.ndimage import gaussian_filter1d
import graphviz as gv
from colour import Color
import os
import glob
from subprocess import CalledProcessError

sensors = ['Health', 'Energy', 'Position X', 'Position Y', 'Closest food x', 'Closest food y']
actions = ['Direction x', 'Direction y', 'Speed']

statistics_path = "../out/build/x64-debug/BioSimulation/data/statistics.txt"
brains_path = "../out/build/x64-debug/BioSimulation/data/brains.txt"

show_statistics = 1
show_brains = 1

amount_of_colors = 100
color_negative = Color(rgb=(1.0, 0, 0))
color_negative_close_zero = Color(rgb=(1.0, 0.43, 0.43))
color_positive = Color(rgb=(0, 1.0, 0))
color_positive_close_zero = Color(rgb=(0.43, 1.0, 0.43))
colors_negative = list(color_negative_close_zero.range_to(color_negative, amount_of_colors))
colors_positive = list(color_positive_close_zero.range_to(color_positive, amount_of_colors))

width_range = 3

label_weights = 1
label_biases = 0

def get_statistics_data(file):
    data = {"Organismamount": [], "Foodamount": [], "Organisms": []}
    file.seek(0, SEEK_END)
    size = file.tell()
    file.seek(0, SEEK_SET)
    extended_statistics = int(file.readline().replace("\n", ""))
    while file.tell() < size:
        line = file.readline()
        if not line == "":
            organisms_size, food = line.replace("\n", "").split(" ")
            data["Organismamount"].append(int(organisms_size))
            data["Foodamount"].append(int(food))

            if extended_statistics:
                organisms = []
                for i in range(int(organisms_size)):
                    line = file.readline()
                    d = [float(x) for x in line.replace("\n", "").split(" ") if not x == '']
                    organisms.append(d)
                data["Organisms"].append(organisms)
    return data

def get_brain_data(file):
    data = []
    file.seek(0, SEEK_END)
    size = file.tell()
    file.seek(0, SEEK_SET)
    while file.tell() < size:
        brain = {}
        brain['Functions'] = file.readline().replace("\n", "").split(" ")
        brain['Input'] = [float(x) for x in file.readline().replace("\n", "").split(" ") if not x == '']
        brain['Hidden'] = [float(x) for x in file.readline().replace("\n", "").split(" ") if not x == '']
        brain['Output'] = [float(x) for x in file.readline().replace("\n", "").split(" ") if not x == '']
        brain['Input->Hidden'] = []
        line = file.readline().replace("\n", "")
        for desc in line.split(";"):
            if desc == '':
                continue
            conn = []
            conn.append(desc.split(" ")[0])
            conn.append(desc.split(" ")[1])
            conn.append(float(desc.split(" ")[2]))
            brain['Input->Hidden'].append(conn)
        brain['Hidden->Hidden'] = []
        line = file.readline().replace("\n", "")
        for desc in line.split(";"):
            if desc == '':
                continue
            conn = []
            conn.append(desc.split(" ")[0])
            conn.append(desc.split(" ")[1])
            conn.append(float(desc.split(" ")[2]))
            brain['Hidden->Hidden'].append(conn)
        brain['Hidden->Output'] = []
        line = file.readline().replace("\n", "")
        for desc in line.split(";"):
            if desc == '':
                continue
            conn = []
            conn.append(desc.split(" ")[0])
            conn.append(desc.split(" ")[1])
            conn.append(float(desc.split(" ")[2]))
            brain['Hidden->Output'].append(conn)
        brain['Input->Output'] = []
        line = file.readline().replace("\n", "")
        for desc in line.split(";"):
            if desc == '':
                continue
            conn = []
            conn.append(desc.split(" ")[0])
            conn.append(desc.split(" ")[1])
            conn.append(float(desc.split(" ")[2]))
            brain['Input->Output'].append(conn)
        data.append(brain)

    return data


def weight_to_color(weight):
    weight = max(min(weight, 1.0), -1.0)
    weight *= amount_of_colors - 1.0
    weight = int(weight)
    return colors_negative[abs(weight)].get_hex_l() if weight < 0 else colors_positive[abs(weight)].get_hex_l()

def weight_to_width(weight):
    weight = max(min(weight, 1.0), -1.0)
    weight = abs(weight)
    weight *= width_range
    return str(weight)


def input_id_to_sensor(id):
    if id >= len(sensors):
        return "NaN"
    return sensors[id]


def output_id_to_action(id):
    if id >= len(actions):
        return "NaN"
    return actions[id]


if __name__=='__main__':
    if show_brains:
        for f in os.listdir("brains/"):
            os.remove("brains/" + f)
        with open(brains_path, "r") as file:
            data = get_brain_data(file)

            file.close()

            i = 0
            for brain in data:
                graph = gv.Digraph("Graph" + str(i), engine='dot', graph_attr={"clusterrank": "global"})

                # Neurons
                neuron_index = 0
                with graph.subgraph(name="input", graph_attr={"rank": "same"}) as sg:
                    for bias in brain['Input']:
                        sg.node("I" + str(neuron_index), input_id_to_sensor(neuron_index) + " " + (str(bias) if label_biases else ""), _attributes={"group": "input"})

                        neuron_index += 1

                # Neurons
                neuron_index = 0
                with graph.subgraph(name="hidden", graph_attr={"rank": "same"}) as sg:
                    for bias in brain['Hidden']:
                        sg.node("H" + str(neuron_index), (str(bias) if label_biases else ""), _attributes={"group": "hidden"})

                        neuron_index += 1

                # Neurons
                neuron_index = 0
                with graph.subgraph(name="output", graph_attr={"rank": "same"}) as sg:
                    for bias in brain['Output']:
                        sg.node("O" + str(neuron_index), output_id_to_action(neuron_index) + " " + (str(bias) if label_biases else ""), _attributes={"group": "Output"})

                        neuron_index += 1

                # Connections
                for conn in brain['Input->Hidden']:
                    graph.edge("I" + conn[0], "H" + conn[1], label=str(conn[2]) if label_weights else "", _attributes={"color": weight_to_color(conn[2]), "penwidth": weight_to_width(conn[2]), "fontcolor": weight_to_color(conn[2])})

                for conn in brain['Hidden->Hidden']:
                    graph.edge("H" + conn[0], "H" + conn[1], label=str(conn[2]) if label_weights else "", _attributes={"color": weight_to_color(conn[2]), "penwidth": weight_to_width(conn[2]), "fontcolor": weight_to_color(conn[2])})

                for conn in brain['Hidden->Output']:
                    graph.edge("H" + conn[0], "O" + conn[1], label=str(conn[2]) if label_weights else "", _attributes={"color": weight_to_color(conn[2]), "penwidth": weight_to_width(conn[2]), "fontcolor": weight_to_color(conn[2])})

                for conn in brain['Input->Output']:
                    graph.edge("I" + conn[0], "O" + conn[1], label=str(conn[2]) if label_weights else "", _attributes={"color": weight_to_color(conn[2]), "penwidth": weight_to_width(conn[2]), "fontcolor": weight_to_color(conn[2])})

                try:
                    graph.render(directory='brains').replace('\\', '/')
                except CalledProcessError as e:
                    print(e)
                i += 1

    if show_statistics:
        with open(statistics_path, "r") as f:
            data = get_statistics_data(f)
            f.close()

            fig, ax = plt.subplots(1, 3)

            # Amount of organisms
            xo = np.arange(0, len(data["Organismamount"]))
            yo = data["Organismamount"]

            yo = gaussian_filter1d(yo, sigma=20)

            ax[0].set_xlabel("Generation")
            ax[0].set_ylabel("Amount of organisms")
            ax[0].plot(xo, yo, "g")

            # Amount of food

            xf = np.arange(0, len(data["Foodamount"]))
            yf = data["Foodamount"]

            yf = gaussian_filter1d(yf, sigma=20)

            ax[1].set_xlabel("Generation")
            ax[1].set_ylabel("Amount of food")
            ax[1].plot(xf, yf, "r")

            # Both
            ax[2].set_xlabel("Generation")
            ax[2].set_ylabel("Amount")
            line1 = ax[2].plot(xf, yf, "r")
            line2 = ax[2].plot(xo, yo, "g")
            ax[2].legend(["Food", "Organisms"])

            plt.show()

