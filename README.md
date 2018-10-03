# CBSSD
> **Community-based semantic subgroup discovery (CBSSD)** is an algorithm for knowledge discovery from
> complex networks. Arbitrary network partitions are used as input for a subgroup discovery step, where explainable rule sets are obtained, associating individual partitions with semantic knowledge.

## Brief algorithm description

> CBSSD uses term lists to query extensive databases of relevant knowledge, in order to construct knowledge graphs.
> In such graphs, distinct communities can emerge, which can be of potential interest to the user. To obtain additional information
> on obtained communities, semantic rule learning using extensive background knowledge can be used to derive sets of community-specific rules, which can yield some new insights about the studied phenomenon.

## IMPORTANT NOTE:
CBSSD is now a part of Py3plex library: example use is given here: https://github.com/SkBlaz/Py3Plex/blob/master/examples/example_CBSSD.py

# Example use on a simple list of epigenetics-related proteins (this was used as a validation dataset). Mind that .obo files can be put in a separate folder, should more than one ontology be used as background knowledge.


```bash
python3 CBSSD.py --step_size 10 --knowledge_graph network_datasets/epigenetics.gpickle --term_list term_lists/epigenetics.list --ontology_BK background_knowledge/goslim_generic.obo --output_BK experimental_evaluation_outputs/epiBK.n3 --n3_samples experimental_evaluation_outputs/epiSam_bmn.n3 --method components --gaf_mapping example_inputs/goa_human.gaf --community_map experimental_evaluation_outputs/partition_epigenetics_bmn_cbssd.txt --rule_output experimental_evaluation_outputs/bmn_epi_rules.json

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
* Install Hedwig by going into the Hedwig folder and write python3 setup.py install
* This is it! Run by python3 CBSSD.py ... (example above) ...

# Citation

```
@InProceedings{10.1007/978-3-319-78680-3_13,
author="{\v{S}}krlj, Bla{\v{z}}
and Kralj, Jan
and Vavpeti{\v{c}}, An{\v{z}}e
and Lavra{\v{c}}, Nada",
editor="Appice, Annalisa
and Loglisci, Corrado
and Manco, Giuseppe
and Masciari, Elio
and Ras, Zbigniew W.",
title="Community-Based Semantic Subgroup Discovery",
booktitle="New Frontiers in Mining Complex Patterns",
year="2018",
publisher="Springer International Publishing",
address="Cham",
pages="182--196",
abstract="Modern data mining algorithms frequently need to address learning from heterogeneous data and knowledge sources, including ontologies. A data mining task in which ontologies are used as background knowledge is referred to as semantic data mining. A special form of semantic data mining is semantic subgroup discovery, where ontology terms are used in subgroup describing rules. We propose to enhance ontology-based subgroup identification by Community-Based Semantic Subgroup Discovery (CBSSD), taking into account also the structural properties of complex networks related to the studied phenomenon. The application of the developed CBSSD approach is demonstrated on two use cases from the field of molecular biology.",
isbn="978-3-319-78680-3"
}

```
