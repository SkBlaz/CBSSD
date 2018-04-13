from py3plex.visualization.multilayer import *
from py3plex.visualization.colors import all_color_names
from py3plex.core import multinet

multilayer_network = multinet.multi_layer_network().load_network("../experimental_networks/intact05.gpickle",directed=False,label_delimiter="---",input_type="gpickle")
hairball_plot(multilayer_network.core_network,"b",layered=False)
plt.show()
