## this script generates a heatmap of term intersections

import argparse
from lib.compare import *

parser_init = argparse.ArgumentParser()
parser_init.add_argument("--enrichment", help="termfile1") # Mandatory
parser_init.add_argument("--cbssd", help="termfile2") # Mandatory
parser_init.add_argument("--out", help="Image heatmap") # Mandatory
parsed = parser_init.parse_args()

## plot the heatmap
print(parsed.enrichment,parsed.cbssd)
compare_lists(parsed.enrichment,parsed.cbssd,parsed.out)
