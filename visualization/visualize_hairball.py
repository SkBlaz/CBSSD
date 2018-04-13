from py3plex.visualization.multilayer import *
from py3plex.visualization.colors import all_color_names
from py3plex.core import multinet

multilayer_network = multinet.multi_layer_network().load_network("../experimental_networks/intact05.gpickle",directed=False,label_delimiter="---",input_type="gpickle")
#network_colors, graph = multilayer_network.get_layers(style="hairball")
#hairball_plot(graph,network_colors)
#plt.show()
