
## benchmark on arbitrary number of lists
out_name=$1;
checkpoint_file="./checkpoints/cx.txt";
for i in ls term_lists/*.list;do

    cx=$(cat $checkpoint_file | grep $i | wc -l)

    if [ $cx != "0" ];then

	echo "Results already exist for $i! Skipping.."
	
    else

	partition_method="components"
	## do a mechanism for saving current progress
	core_name="goslim_"$(echo "$i" | mawk '{split($1,a,"/");split(a[2],b,".");print b[1]}')"_$partition_method";
	file_path=$(echo "$i");
	echo "$core_name"
	
	## epigenetics - CBSSD + enrichment #######################################################
	echo "python3 CBSSD.py --step_size 5 --knowledge_graph network_datasets/"$core_name".gpickle --term_list $file_path --ontology_BK background_knowledge/goslim_generic.obo --output_BK experimental_evaluation_outputs/"$core_name"BK.n3 --n3_samples experimental_evaluation_outputs/"$core_name"Sam_bmn.n3 --gaf_mapping example_inputs/goa_human.gaf --method "$partition_method" --community_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt --rule_output experimental_evaluation_outputs/bmn_"$core_name"_rules.json" |sh

	## compute enriched terms
	echo "python3 term_enrichment_CBSSD.py --gaf_filename example_inputs/goa_human.gaf --partition_mappings experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt --outfile experimental_evaluation_outputs/bmn_"$core_name"_term_enrichment.txt" | sh

	## evaluate
	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_"$core_name"_rules.json --result_type rules --partition_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt >> evaluation_results/"$out_name".txt" |sh

	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_"$core_name"_term_enrichment.txt --result_type terms --partition_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt >> evaluation_results/"$out_name".txt" | sh


	## proteome part
	echo "python3 CBSSD.py --knowledge_graph network_datasets/intact02.gpickle --term_list $file_path --ontology_BK background_knowledge/goslim_generic.obo --output_BK experimental_evaluation_outputs/"$core_name"BK.n3 --n3_samples experimental_evaluation_outputs/"$core_name"Sam_intact.n3 --gaf_mapping example_inputs/goa_human.gaf --method "$partition_method" --community_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt --rule_output experimental_evaluation_outputs/intact_"$core_name"_rules.json" |sh

	## compute enriched terms
	echo "python3 term_enrichment_CBSSD.py --gaf_filename example_inputs/goa_human.gaf --partition_mappings experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt --outfile experimental_evaluation_outputs/intact_"$core_name"_term_enrichment.txt" |sh

	## evaluate
	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/intact_"$core_name"_rules.json --result_type rules --partition_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt >> evaluation_results/"$out_name".txt" |sh

	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/intact_"$core_name"_term_enrichment.txt --result_type terms --partition_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt >> evaluation_results/"$out_name".txt" |sh



	#######################
	# WHOLE GO
	#######################


        core_name="go_"$(echo "$i" | mawk '{split($1,a,"/");split(a[2],b,".");print b[1]}');
	file_path=$(echo "$i");
	echo "$core_name"
	
	## epigenetics - CBSSD + enrichment #######################################################
	echo "python3 CBSSD.py --step_size 5 --knowledge_graph network_datasets/"$core_name".gpickle --term_list $file_path --ontology_BK background_knowledge/go.obo --output_BK experimental_evaluation_outputs/"$core_name"BK.n3 --n3_samples experimental_evaluation_outputs/"$core_name"Sam_bmn.n3 --gaf_mapping example_inputs/goa_human.gaf --method "$partition_method" --community_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt --rule_output experimental_evaluation_outputs/bmn_"$core_name"_rules.json" |sh

	## compute enriched terms
	echo "python3 term_enrichment_CBSSD.py --gaf_filename example_inputs/goa_human.gaf --partition_mappings experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt --outfile experimental_evaluation_outputs/bmn_"$core_name"_term_enrichment.txt" | sh

	## evaluate
	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_"$core_name"_rules.json --result_type rules --partition_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt >> evaluation_results/"$out_name".txt" |sh

	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/bmn_"$core_name"_term_enrichment.txt --result_type terms --partition_map experimental_evaluation_outputs/partition_"$core_name"_bmn_cbssd.txt >> evaluation_results/"$out_name".txt" | sh


	## proteome part
	echo "python3 CBSSD.py --knowledge_graph network_datasets/intact02.gpickle --term_list $file_path --ontology_BK background_knowledge/go.obo --output_BK experimental_evaluation_outputs/"$core_name"BK.n3 --n3_samples experimental_evaluation_outputs/"$core_name"Sam_intact.n3 --gaf_mapping example_inputs/goa_human.gaf --method "$partition_method" --community_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt --rule_output experimental_evaluation_outputs/intact_"$core_name"_rules.json" |sh

	## compute enriched terms
	echo "python3 term_enrichment_CBSSD.py --gaf_filename example_inputs/goa_human.gaf --partition_mappings experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt --outfile experimental_evaluation_outputs/intact_"$core_name"_term_enrichment.txt" |sh

	## evaluate
	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/intact_"$core_name"_rules.json --result_type rules --partition_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt >> evaluation_results/"$out_name".txt" |sh

	echo "python3 evaluate_cbssd.py --input_gaf example_inputs/goa_human.gaf --enrichment_results experimental_evaluation_outputs/intact_"$core_name"_term_enrichment.txt --result_type terms --partition_map experimental_evaluation_outputs/partition_"$core_name"_intact_cbssd.txt >> evaluation_results/"$out_name".txt" |sh

	#echo "$i\n" >> $checkpoint_file;

    fi
done
