# CBSSD
> **Community-based semantic subgroup discovery (CBSSD)** is an algorithm for knowledge discovery from
> arbitrary term lists. Current version is targeted at biological term lists, e.g. gene or protein lists, yet
> the methodology can be applied to any other problem as well.

## Brief algorithm description

> CBSSD uses term lists to query extensive databases of relevant knowledge, in order to construct knowledge graphs.
> In such graphs, distinct communities can emerge, which can be of potential interest to the user. To obtain additional information
> on obtained communities, semantic rule learning using extensive background knowledge can be used to derive sets of community-specific rules, which can yield some new insights about the studied phenomenon.


## Example use on a simple list of epigenetics-related proteins (this was used as a validation dataset). Mind that .obo files can be put in a separate folder, should more than one ontology be used as background knowledge.


```bash

python3 CBSSD.py --step_size 10 --knowledge_graph example_outputs/epigenetics.gpickle --term_list example_inputs/epigenetics.list --ontology_BK example_inputs --output_BK example_outputs/epiBK.n3 --n3_samples example_outputs/epiSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/EpiCom.txt --rule_output example_outputs/Epirules.txt 

```

### Brief parameter descriptions:


* step_size = size of terms used as a query to the BioMine graph crawler
* knowledge_graph = saved result graph from BioMine crawl
* term_list = list of input terms (MAIN INPUT)
* ontology_BK = background knowledge in .obo format
* output_BK = knowledge in .n3 format, used in this process
* n3_samples = query dataset derived from terms
* gaf_mapping = map from names to GO terms for SSD
* community_map = nodes with assigned communities
* rule_output = main result, rules for individual communities

### Installation

* Compile InfoMap (folder InfoMap, type make for example)
* Python libraries: Networkx, Rdflib, Numpy, matplotlib (optional)
* Install Hedwig by going into the Hedwig folder and write python2 setup.py install - follow instructions
* This is it! Run by python3 CBSSD.py ... (example above) ...

# Citation

```
@article{vskrljcommunity,
  title={Community-based semantic subgroup discovery},
  author={{\v{S}}krlj, Blaz and Vavpetic, Anze and Kralj, Jan and Lavrac, Nada}
}
```

