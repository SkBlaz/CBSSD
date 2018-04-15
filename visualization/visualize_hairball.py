from py3plex.visualization.multilayer import *
from py3plex.visualization.colors import all_color_names
from py3plex.core import multinet
import argparse


parser_init = argparse.ArgumentParser()
parser_init.add_argument("--input_graph", help="Load graph file")
parser = parser_init.parse_args()

multilayer_network = multinet.multi_layer_network().load_network(parser.input_graph,directed=False,label_delimiter="---",input_type="gpickle")
hairball_plot(multilayer_network.core_network,"b",layered=False,legend=False,layout_parameters={'iterations':100})
plt.show()
