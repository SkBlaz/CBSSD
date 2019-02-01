# CBSSD
> **Community-based semantic subgroup discovery (CBSSD)** is an algorithm for knowledge discovery from
> complex networks. Arbitrary network partitions are used as input for a subgroup discovery step, where explainable rule sets are obtained, associating individual partitions with semantic knowledge.

## Brief algorithm description

> CBSSD uses term lists to query extensive databases of relevant knowledge, in order to construct knowledge graphs.
> In such graphs, distinct communities can emerge, which can be of potential interest to the user. To obtain additional information
> on obtained communities, semantic rule learning using extensive background knowledge can be used to derive sets of community-specific rules, which can yield some new insights about the studied phenomenon.

## IMPORTANT NOTE:
CBSSD is now a part of Py3plex library: example use is given here: https://github.com/SkBlaz/Py3Plex/blob/master/examples/example_CBSSD.py

### Example use on a simple list of epigenetics-related proteins (this was used as a validation dataset). Mind that .obo files can be put in a separate folder, should more than one ontology be used as background knowledge.


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
@Article{Škrlj2019,
author="{\v{S}}krlj, Bla{\v{z}}
and Kralj, Jan
and Lavra{\v{c}}, Nada",
title="CBSSD: community-based semantic subgroup discovery",
journal="Journal of Intelligent Information Systems",
year="2019",
month="Jan",
day="26",
abstract="Modern data mining algorithms frequently need to address the task of learning from heterogeneous data, including various sources of background knowledge. A data mining task where ontologies are used as background knowledge in data analysis is referred to as semantic data mining. A specific semantic data mining task is semantic subgroup discovery: a rule learning approach enabling ontology terms to be used in subgroup descriptions learned from class labeled data. This paper presents Community-Based Semantic Subgroup Discovery (CBSSD), a novel approach that advances ontology-based subgroup identification by exploiting the structural properties of induced complex networks related to the studied phenomenon. Following the idea of multi-view learning, using different sources of information to obtain better models, the CBSSD approach can leverage different types of nodes of the induced complex network, simultaneously using information from multiple levels of a biological system. The approach was tested on ten data sets consisting of genes related to complex diseases, as well as core metabolic processes. The experimental results demonstrate that the CBSSD approach is scalable, applicable to large complex networks, and that it can be used to identify significant combinations of terms, which can not be uncovered by contemporary term enrichment analysis approaches.",
issn="1573-7675",
doi="10.1007/s10844-019-00545-0",
url="https://doi.org/10.1007/s10844-019-00545-0"
}


```
