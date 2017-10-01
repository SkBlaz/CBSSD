## example runs of the CBSSD!

## epigenetics
python3 CBSSD.py --step_size 2 --knowledge_graph example_outputs/epigenetics.gpickle --term_list example_inputs/epigenetics.list --ontology_BK example_inputs --output_BK example_outputs/epiBK.n3 --n3_samples example_outputs/epiSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/EpiCom.txt --rule_output example_outputs/Epirules.txt

## diabetes
python3 CBSSD.py --step_size 2 --knowledge_graph example_outputs/diabetes.gpickle --term_list example_inputs/uniprot-diabetes.list --ontology_BK example_inputs --output_BK example_outputs/diabBK.n3 --n3_samples example_outputs/diabSam.n3 --gaf_mapping example_inputs/goa_human.gaf --community_map example_outputs/diabcom.txt --rule_output example_outputs/diabrules.txt
