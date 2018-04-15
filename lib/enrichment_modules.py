##### this pyton code enables enrichment calculation from graph results from previous step

## this is to calculate enrichment scores

from scipy.stats import fisher_exact
from collections import defaultdict

def read_SCOP_terms(filename):
    term_counts = defaultdict(list)
    candidate_terms = set()
    scop_filter = 2
    
    with open(filename) as scopfile:
        for line in scopfile:
            if not line.startswith("#"):
                if len(line.split()) == 6:
                    pdb,chain,scop_level = line.split()[4],line.split()[5].split(":")[0],".".join(line.split()[2].split(".")[0:scop_filter])
                    candidate_terms.add(scop_level)
                    term_counts[pdb+chain].append(scop_level)

    return (term_counts,list(candidate_terms))
    

def read_pdb_GO(filename):

    ## this function reads the base pdb GO file and uses it as the seed for further enrichment analysis
    
    term_counts = defaultdict(list)
    candidate_terms = set()
    
    if filename.endswith(".tsv"):    
        with open(filename) as fn:
            for line in fn:
                try:
                    pdbID,GOterm = "".join(line.split()[0:2]),line.split()[5]
                    term_counts[pdbID].append(GOterm)                        
                    candidate_terms.add(GOterm) ## this is the candidate base to search through
                except:
                    pass
                
    elif filename.endswith(".gz"):
        import gzip
        with gzip.open(filename,'rt') as fn:
            for line in fn:
                try:
                    pdbID,GOterm = "".join(line.split()[0:2]),line.split()[5]
                    term_counts[pdbID].append(GOterm)                        
                    candidate_terms.add(GOterm) ## this is the candidate base to search through
                except:
                    pass

        
    return (term_counts,list(candidate_terms))

def preprocess_pfam(term):

    return term.replace("b'","")

def read_pfam_terms(filename):

    term_counts = defaultdict(list)
    candidate_terms = set()

    if filename.endswith(".tsv"):
        with open(filename) as fn:
            for line in fn:
                try:
                    pdbID,pfamterm = "".join(line.split()[0:2]),line.split()[3]
                    term_counts[pdbID].append(GOterm)                        
                    candidate_terms.add(GOterm)
                except:
                    pass
                
    elif filename.endswith(".gz"):
        import gzip
        with gzip.open(filename,'rt') as fn:
            for line in fn:
                a = line.split()
                pdbID = a[0]+a[1]
                GOterm = a[3]
                try:
                    term_counts[pdbID].append(GOterm)                        
                    candidate_terms.add(GOterm)
                except:
                    pass
        
    return (term_counts,list(candidate_terms))


def calculate_pval(query,dbsource,term,size=False):

    ## this calculates p value
    
    qcount = 0
    dbcount = 0
    
    for q in query:
        if q in dbsource[q]:
            qcount+=1
            
    for v in dbsource.values():
        if term in v:
            dbcount +=1

    query_counts = [qcount, len(query)-qcount]
    pop_counts = [dbcount, len(dbsource)-dbcount]    
    p_value = fisher_exact([query_counts,pop_counts])[1]

    return p_value

def multiple_test_correction(input_dataset):
    from statsmodels.sandbox.stats.multicomp import multipletests
    pvals = defaultdict(list)
    with open(input_dataset) as ods:
        for line in ods:
            try:
                component, term, pval = line.split()
                pvals[component].append((term,pval))
            except:
                pass

    print ("Component_by_size PFAM_term pvalue")
    for key, values in pvals.items():
        tmpP = [float(val[1]) for val in values]
        termN = [val[0] for val in values]
        significant, pvals, sidak, bonf = multipletests(tmpP,method="hs",is_sorted=False,returnsorted=False)

        ## Holm Sidak        
        output = zip(termN,significant,pvals,tmpP)
        for term,significant,pval,tmp in output:
            if (significant == True):
                print (key,term,significant,tmp,pval)

 #   print(pvals)
    
if __name__ == "__main__":
    print("Starting enrichment analysis..")

    ## perform enrichment analysis
    import multiprocessing as mp
    import random
    from statsmodels.sandbox.stats.multicomp import multipletests

    random.seed(123)
    if args.termbase:

        if args.termbase == "GO":
            term_dataset, termBase = read_pdb_GO(args.termfile)
        elif args.termbase == "PFAM":
            term_dataset, termBase = read_pfam_terms(args.termfile)
        elif args.termbase == "SCOP":
            term_dataset, termBase = read_SCOP_terms(args.termfile)
        else:
            exit

        rep_chains = ["".join(x.split("_")[1:3]) for x in G.nodes()]

        ## remove items not present in the network

        to_del = []
        for k in term_dataset.keys():
            if k not in rep_chains:
                to_del.append(k)

        print("Removing",len(to_del),"terms.")

        for x in to_del:
            del term_dataset[x]

        print("Read the terms..\n","termbase size:",len(termBase))

        if args.enrichment_type == "community":
            print ("Communuty enrichment..")
            if args.community_file:
                components = defaultdict(list)
                with open(args.community_file) as cf:
                    for line in cf:
                        node, module = line.strip().split()
                        components[module].append(node)
            else:                    
                components, individual = run_infomap(G)

            components = sorted(components.values(),key=len,reverse=True)

        elif args.enrichment_type == "components":
            print ("Component enrichment..")
            components = sorted(nx.connected_components(G),key=len,reverse=True)

        result_terms = {}
        component_size = 0

        columns2 = ['observation','term','pval','corrected_pval','significant']
        pvals = pd.DataFrame(columns=columns2)

        ## this is to be done on GPU, theoretically..

        print ("Beginning enrichment..")
        for enum,component in enumerate(components):

            component = ["".join(x.split("_")[1:3]) for x in component]
            result_terms[enum]={}

            def parallel_enrichment(term):
                term = termBase[term]
                pval = calculate_pval(component,term_dataset,term)
                return {'observation' : enum,'term' : term,'pval' : pval}

            pool = mp.Pool(mp.cpu_count())
            inputs = [x for x in range(0,len(termBase))]
            results = pool.map(parallel_enrichment,inputs)

            pool.close()
            pool.join() ## this merges processes

            tmpframe = pd.DataFrame(columns=['observation','term','pval'])
            tmpframe = tmpframe.append(results,ignore_index=True)
            significant, p_adjusted, sidak, bonf = multipletests(tmpframe['pval'],method="fdr_bh",is_sorted=False, returnsorted=False, alpha=0.01)
            tmpframe['corrected_pval'] = pd.Series(p_adjusted)
            tmpframe['significant'] = pd.Series(significant)
            tmpframe = tmpframe[tmpframe['significant'] == True]
            if not tmpframe.empty:
                print(tmpframe)
            with open(args.enrichment_outfile, 'a') as f:
                tmpframe.to_csv(f, header=False)
