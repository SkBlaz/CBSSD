/*
 * InfomapBase.h
 *
 *  Created on: 23 feb 2015
 *      Author: Daniel
 */

#include "InfomapBase.h"
#include <string>
#include <iostream>
#include <sstream>
#include "InfoNode.h"
#include "InfoEdge.h"
#include <vector>
#include <deque>
#include "FlowData.h"
#include "MapEquation.h"
#include "../utils/Log.h"
#include "../utils/infomath.h"
#include "../utils/Date.h"
#include "../utils/Stopwatch.h"
#include <iomanip>
#include <limits>
#include "InfomapConfig.h"
#include "../utils/exceptions.h"
#include "ClusterMap.h"
#include <map>
#include <set>
// #include <type_traits>
#include "PartitionQueue.h"
#include <cstdlib> // abs
#include <limits>
// #include "StateNetwork.h"
#include "../io/Network.h"
#ifdef _OPENMP
#include <omp.h>
#endif

namespace infomap {

// ===================================================
// IO
// ===================================================

std::ostream& operator<<(std::ostream& out, const InfomapBase& infomap)
{
	return infomap.toString(out);
}

std::ostream& InfomapBase::toString(std::ostream& out) const
{
	return out << numTopModules() << " top modules.";
}

// ===================================================
// Getters
// ===================================================

const Network& InfomapBase::network() const {
	return m_network;
}

Network& InfomapBase::network() {
	return m_network;
}

InfoNode& InfomapBase::root() {
	return m_root;
}

const InfoNode& InfomapBase::root() const {
	return m_root;
}

unsigned int InfomapBase::numLeafNodes() const
{
	return m_leafNodes.size();
}

const std::vector<InfoNode*>& InfomapBase::leafNodes() const
{
	return m_leafNodes;
}

unsigned int InfomapBase::numTopModules() const
{
	return m_root.childDegree();
}

unsigned int InfomapBase::numNonTrivialTopModules() const
{
	return m_numNonTrivialTopModules;
}

bool InfomapBase::haveModules() const
{
	return !m_root.isLeaf() && !m_root.firstChild->isLeaf();
}

unsigned int InfomapBase::numLevels() const
{
	unsigned int depth = 0;
	InfoNode* n = m_root.firstChild;
	while (n != nullptr) {
		++depth;
		n = n->firstChild;
	}
	return depth;
}

double InfomapBase::getHierarchicalCodelength() const
{
	return m_hierarchicalCodelength;
}

InfomapBase& InfomapBase::getInfomap(InfoNode& node) {
	return node.getInfomap();
}

InfomapBase& InfomapBase::getSubInfomap(InfoNode& node) {
	return getInfomap(node)
		.setIsMain(false)
		.setSubLevel(m_subLevel + 1)
		.setNonMainConfig(*this);
}

InfomapBase& InfomapBase::getSuperInfomap(InfoNode& node) {
	return getInfomap(node)
		.setIsMain(false)
		.setSubLevel(m_subLevel + SUPER_LEVEL_ADDITION)
		.setNonMainConfig(*this);
}

InfomapBase& InfomapBase::setIsMain(bool isMain) {
	m_isMain = isMain;
	return *this;
}

InfomapBase& InfomapBase::setSubLevel(unsigned int level) {
	m_subLevel = level;
	return *this;
}


bool InfomapBase::isTopLevel() const
{
	return (m_subLevel & (SUPER_LEVEL_ADDITION - 1)) == 0;
}

bool InfomapBase::isSuperLevelOnTopLevel() const
{
	return m_subLevel == SUPER_LEVEL_ADDITION;
}

bool InfomapBase::isMainInfomap() const
{
	// return m_root.owner == nullptr || m_root.owner->isRoot();
	return m_isMain;
}

bool InfomapBase::haveMemory() const
{
//	return std::is_base_of<M2InfoNode<Node>, Node>::value;
	return false;
}

// template<>
// bool InfomapBase<M1InfoNode>::haveMemory() const { return false; }
//
// template<>
// bool InfomapBase<M2InfoNode>::haveMemory() const { return true; }

bool InfomapBase::haveHardPartition() const
{
	return !m_originalLeafNodes.empty();
}


std::vector<InfoNode*>& InfomapBase::activeNetwork() const
{
	return *m_activeNetwork;
}

// ===================================================
// Run
// ===================================================

void InfomapBase::run()
{
	if (!isMainInfomap()) {
		runPartition();
		return;
	}
	
	Network network(*this);

	std::string filename = this->networkFile;

	network.readInputData(filename);

	run(network);
}

void InfomapBase::run(Network& network)
{
	if (!isMainInfomap())
		throw InternalOrderError("Can't run a non-main Infomap with an input network");

	network.calculateFlow();
	
	initNetwork(network);

	if (numLeafNodes() == 0)
		throw DataDomainError("No nodes to partition");
	
	if (haveMemory())
		Log(2) << "Run Infomap with memory..." << std::endl;
	else
		Log(2) << "Run Infomap..." << std::endl;
	
	std::ostringstream bestSolutionStatistics;
	unsigned int bestNumLevels = 0;
	double bestHierarchicalCodelength = std::numeric_limits<double>::max();
	std::deque<double> codelengths;

	unsigned int numTrials = this->numTrials;

	for (unsigned int i = 0; i < numTrials; ++i)
	{
		removeModules();
		auto startDate = Date();
		Stopwatch timer(true);

		if (isMainInfomap())
		{
			Log() << "\n";
			Log() << "================================================\n";
			Log() << "Trial " << (i + 1) << "/" << numTrials << " starting at" << startDate << "\n";
			Log() << "================================================\n";

			if (this->clusterDataFile != "")
				initPartition(this->clusterDataFile, this->clusterDataIsHard);
		}

		runPartition();

		if (haveHardPartition())
			restoreHardPartition();
		
		if (isMainInfomap()) {
			auto endDate = Date();
			Log() << "\n=> Trial " << (i + 1) << "/" << numTrials <<
				" finished in " << timer.getElapsedTimeInMilliSec() << "s with codelength " << m_hierarchicalCodelength << "\n";
			codelengths.push_back(m_hierarchicalCodelength);
			if (m_hierarchicalCodelength < bestHierarchicalCodelength - 1e-10) {
				bestSolutionStatistics.clear();
				bestSolutionStatistics.str("");
				bestNumLevels = printPerLevelCodelength(bestSolutionStatistics);
				bestHierarchicalCodelength = m_hierarchicalCodelength;
				writeResult();
			}
		}
	}
	if (isMainInfomap())
	{
		Log() << "\n\n";
		Log() << "================================================\n";
		Log() << "Summary after " << numTrials << (numTrials > 1 ? " trials\n" : " trial\n");
		Log() << "================================================\n";
		if (codelengths.size() > 1) {
			Log() << std::fixed << std::setprecision(9);
			double averageCodelength = 0.0;
			double minCodelength = codelengths[0];
			double maxCodelength = codelengths[0];
			Log() << "Codelengths: [";
			for (auto codelength : codelengths) {
				Log() << codelength << ", ";
				averageCodelength += codelength;
				minCodelength = std::min(minCodelength, codelength);
				maxCodelength = std::max(maxCodelength, codelength);
			}
			averageCodelength /= numTrials;
			Log() << "\b\b]\n";
			Log() << "[min, average, max] codelength: [" <<
					minCodelength << ", " << averageCodelength << ", " << maxCodelength << "]\n\n";
		}

		Log() << "Best end modular solution in " << bestNumLevels << " levels";
		if (bestHierarchicalCodelength > m_oneLevelCodelength)
			Log() << " (warning: worse than one-level solution)";
		Log() << ":" << std::endl;
		Log() << bestSolutionStatistics.str() << std::endl;
	}
}


// ===================================================
// Run: Init: *
// ===================================================

InfomapBase& InfomapBase::initNetwork(StateNetwork& network)
{
	if (network.numNodes() == 0)
		throw DataDomainError("No nodes in network");
	this->setConfig(network.getConfig());
	Log() << "Init " << (this->directedEdges ? "directed" : "undirected") << " network...\n";
	generateSubNetwork(network);

	init();
	return *this;
}

InfomapBase& InfomapBase::initNetwork(InfoNode& parent, bool asSuperNetwork)
{
	generateSubNetwork(parent);

	if (asSuperNetwork)
		transformNodeFlowToEnterFlow(m_root);

	init();
	return *this;
}

InfomapBase& InfomapBase::initPartition(std::string clusterDataFile, bool hard)
{
	// using NodeKey = typename std::remove_cv<decltype(std::declval<Node>().key)>::type;
	// ClusterMap<NodeKey> clusterMap;
	// clusterMap.readClusterData(clusterDataFile);

	// std::map<NodeKey, unsigned int> clusterIds = clusterMap.clusterIds();

	// // Group on cluster id
	// std::map<unsigned int, std::set<NodeKey>> keysGroupedOnClusterId;
	// for (auto it = clusterIds.begin(); it != clusterIds.end(); ++it){
	// 	keysGroupedOnClusterId[it->second].insert(it->first);
	// }

	// // Create map from key to leaf node index
	// std::map<NodeKey, unsigned int> keyToNodeIndex;
	// for (unsigned int i = 0; i < m_leafNodes.size(); ++i) {
	// 	keyToNodeIndex[get(*m_leafNodes[i]).key] = i;
	// }

	// // Create vectors of clusters
	// std::vector<std::vector<unsigned int>> clusters(keysGroupedOnClusterId.size());
	// unsigned int numNodesNotFound = 0;
	// unsigned int clusterIndex = 0;
	// for (auto& clusterToKeys : keysGroupedOnClusterId) {
	// 	auto& keys = clusterToKeys.second;
	// 	clusters[clusterIndex].reserve(keys.size());
	// 	for (auto& key : keys) {
	// 		auto it = keyToNodeIndex.find(key);
	// 		if (it == keyToNodeIndex.end()) {
	// 			++numNodesNotFound;
	// 		}
	// 		else {
	// 			unsigned int nodeIndex = it->second;
	// 			clusters[clusterIndex].push_back(nodeIndex);
	// 		}
	// 	}
	// 	++clusterIndex;
	// }
	// if (numNodesNotFound > 0)
	// 	Log() << "(Warning: " << numNodesNotFound << " nodes not found in network.) ";

	// return initPartition(clusters, hard);
	std::vector<std::vector<unsigned int>> clusters;
	return initPartition(clusters, hard);
}

InfomapBase& InfomapBase::initPartition(std::vector<std::vector<unsigned int>>& clusters, bool hard)
{
	Log(3) << "\n -> Init " << (hard? "hard" : "soft") << " partition... " << std::flush;
	unsigned int numNodes = numLeafNodes();
	// Get module indices from the merge structure
	std::vector<unsigned int> modules(numNodes);
	std::vector<unsigned int> selectedNodes(numNodes, 0);
	unsigned int moduleIndex = 0;
	for (auto& cluster : clusters) {
		for (auto id : cluster) {
			++selectedNodes[id];
			modules[id] = moduleIndex;
		}
		++moduleIndex;
	}
	// Put non-selected nodes in its own module
	for (unsigned int i = 0; i < numNodes; ++i) {
		if (selectedNodes[i] == 0) {
			modules[i] = moduleIndex;
			++moduleIndex;
		}
	}
	return initPartition(modules, hard);
}

InfomapBase& InfomapBase::initPartition(std::vector<unsigned int>& modules, bool hard)
{
//	Log(3) << "Init " << (hard? "hard" : "soft") << " partition..." << std::endl;
	removeModules();
	setActiveNetworkFromLeafs();
	initPartition(); //TODO: confusing same name, should be able to init default without arguments here too
	moveActiveNodesToPredifinedModules(modules);
	consolidateModules(false);

	if (hard) {
		// Save the original network
		m_originalLeafNodes.swap(m_leafNodes);

		// Use the pre-partitioned network as the new leaf network
		unsigned int numNodesInNewNetwork = numTopModules();
		m_leafNodes.resize(numNodesInNewNetwork);
		unsigned int nodeIndex = 0;
		unsigned int numLinksInNewNetwork = 0;
		for (InfoNode& node : m_root) {
			m_leafNodes[nodeIndex] = &node;
			// Collapse children
			node.collapseChildren();
			numLinksInNewNetwork += node.outDegree();
			++nodeIndex;
		}

		Log(1) << "\n -> Hard-partitioned the network to " << numNodesInNewNetwork << " nodes and " << numLinksInNewNetwork << " links with codelength " << *this << std::endl;
	}
	else {
		Log(1) << "\n -> Initiated to codelength " << *this << " in " << numTopModules() << " top modules." << std::endl;
	}

	return *this;
}

void InfomapBase::generateSubNetwork(StateNetwork& network)
{
	unsigned int numNodes = network.numNodes();

	Log() << "Generate network with " << numNodes << " nodes and " << network.numLinks() << " links..." << std::endl;

	m_leafNodes.reserve(numNodes);
	double sumNodeFlow = 0.0;
	std::map<unsigned int, unsigned int> nodeIndexMap;
	for (auto& nodeIt : network.nodes()) {
		auto& networkNode = nodeIt.second;
		InfoNode* node = new InfoNode(networkNode.flow, networkNode.id, networkNode.physicalId);
		sumNodeFlow += networkNode.flow;
		m_root.addChild(node);
		nodeIndexMap[networkNode.id] = m_leafNodes.size();
		m_leafNodes.push_back(node);
	}
	m_root.data.flow = sumNodeFlow;
	m_calculateEnterExitFlow = true;

	if (std::abs(sumNodeFlow - 1.0) > 1e-10)
		Log() << "Warning, total flow on nodes differs from 1.0 by " << sumNodeFlow - 1.0 << "." << std::endl;

	unsigned int numLinksIgnored = 0;
	for (auto& linkIt : network.nodeLinkMap()) {
		unsigned int linkSourceId = linkIt.first.id;
		unsigned int sourceIndex = nodeIndexMap[linkSourceId];
		const auto& subLinks = linkIt.second;
		for (auto& subIt : subLinks)
		{
			unsigned int linkTargetId = subIt.first.id;
			unsigned int targetIndex = nodeIndexMap[linkTargetId];
			if (sourceIndex == targetIndex) {
				++numLinksIgnored;
			}
			else {
				auto& linkData = subIt.second;
				m_leafNodes[sourceIndex]->addOutEdge(*m_leafNodes[targetIndex], linkData.weight, linkData.flow);
				// Log() << linkSourceId << " (" << sourceIndex << ") -> " << linkTargetId << " (" << targetIndex << ")"
				// << ", weight: " << linkData.weight << ", flow: " << linkData.flow << "\n";
			}
		}
	}
	
	if (numLinksIgnored > 0) {
//		Log(1) << numLinksIgnored << " links with ~0 flow ignored -> " << network.getFlowLinks().size() - numLinksIgnored << " links." << std::endl;
		Log() << numLinksIgnored << " self-links ignored -> " << network.numLinks() - numLinksIgnored << " links." << std::endl;
	}
}

void InfomapBase::generateSubNetwork(InfoNode& parent)
{
	// Store parent module
	m_root.owner = &parent;
	m_root.data = parent.data;

	unsigned int numNodes = parent.childDegree();
	m_leafNodes.resize(numNodes);

	Log(1) << "Generate sub network with " << numNodes << " nodes..." << std::endl;

	unsigned int childIndex = 0;
	for (InfoNode& node : parent) {
		InfoNode* clonedNode = new InfoNode(node);
		clonedNode->initClean();
		m_root.addChild(clonedNode);
		node.index = childIndex; // Set index to its place in this subnetwork to be able to find edge target below
		m_leafNodes[childIndex] = clonedNode;
		++childIndex;
	}

	InfoNode* parentPtr = &parent;
	// Clone edges
	for (InfoNode& node : parent)
	{
		for (EdgeType* e : node.outEdges())
		{
			EdgeType& edge = *e;
			// If neighbour node is within the same module, add the link to this subnetwork.
			if (edge.target.parent == parentPtr)
			{
				m_leafNodes[edge.source.index]->addOutEdge(*m_leafNodes[edge.target.index], edge.data.weight, edge.data.flow);
			}
		}
	}
}

void InfomapBase::init()
{
	Log(3) << "InfomapBase::init()..." << std::endl;

	if (m_calculateEnterExitFlow && isMainInfomap())
		initEnterExitFlow();

	initNetwork();

	Log() << "Calculating one-level codelength... " << std::flush;
	m_oneLevelCodelength = calcCodelength(m_root);
	Log() << "done!\n -> One-level codelength: " << m_oneLevelCodelength << std::endl;
}


// ===================================================
// Run: *
// ===================================================

void InfomapBase::runPartition()
{
	if (this->twoLevel)
		partition();
	else
		hierarchicalPartition();
}

void InfomapBase::hierarchicalPartition()
{
	Log(1) << "Hierarchical partition..." << std::endl;

	partition();

	if (numTopModules() == 1 || numTopModules() == numLeafNodes()) {
		Log(1) << "Trivial partition, skip search for hierarchical solution.\n";
		return;
	}

	findHierarchicalSuperModules();
//	findHierarchicalSuperModulesFast(superLevelLimit);

	if (this->onlySuperModules) {
		removeSubModules(true);
		return;
	}

	if (this->fastHierarchicalSolution >= 2) {
		// Calculate individual module codelengths and store on the modules
		calcCodelengthOnTree(false);
		return;
	}

	if (this->fastHierarchicalSolution == 0) {
		removeSubModules(true);
	}

	recursivePartition();
}

void InfomapBase::partition()
{
	auto oldPrecision = Log::precision();
	Log(0,0) << "Two-level compression: " << std::setprecision(2) << std::flush;
	Log(1) << "Trying to find modular structure... \n";
	double oldCodelength = m_oneLevelCodelength;

	m_tuneIterationIndex = 0;
	findTopModulesRepeatedly(this->levelAggregationLimit);

	double newCodelength = getCodelength();
	double compression = (oldCodelength - newCodelength)/oldCodelength;
	Log(0,0) << (compression * 100) << "% " << std::flush;
	oldCodelength = newCodelength;

	bool doFineTune = true;
	bool coarseTuned = false;
	while (numTopModules() > 1 && (m_tuneIterationIndex + 1) != this->tuneIterationLimit)
	{
		++m_tuneIterationIndex;
		if (doFineTune)
		{
			Log(3) << "\n";
			Log(1,3) << "Fine tune... " << std::flush;
			unsigned int numEffectiveLoops = fineTune();
			if (numEffectiveLoops > 0) {
				Log(1,3) << " -> done in " << numEffectiveLoops << " effective loops to codelength " << *this <<
						" in " << numTopModules() << " modules." << std::endl;
				// Continue to merge fine-tuned modules
				findTopModulesRepeatedly(this->levelAggregationLimit);
			}
			else {
				Log(1,3) << "no improvement." << std::endl;
			}
		}
		else
		{
			coarseTuned = true;
			if (!this->noCoarseTune)
			{
				Log(3) << "\n";
				Log(1,3) << "Coarse tune... " << std::flush;
				unsigned int numEffectiveLoops = coarseTune();
				if (numEffectiveLoops > 0) {
					Log(1,3) << "done in " << numEffectiveLoops << " effective loops to codelength " << *this <<
							" in " << numTopModules() << " modules." << std::endl;
					// Continue to merge fine-tuned modules
					findTopModulesRepeatedly(this->levelAggregationLimit);
				}
				else {
					Log(1,3) << "no improvement." << std::endl;
				}
			}
		}
		newCodelength = getCodelength();
		compression = (oldCodelength - newCodelength)/oldCodelength;
		bool isImprovement = newCodelength <= oldCodelength - this->minimumCodelengthImprovement;
		if (!isImprovement) {
			// Make sure coarse-tuning have been tried
			if (coarseTuned)
				break;
		}
		else {
			oldCodelength = newCodelength;
		}
		Log(0,0) << (compression * 100) << "% " << std::flush;
		doFineTune = !doFineTune;
	}

	Log(0,0) << std::setprecision(oldPrecision) << std::endl;
	Log() << "Partitioned to codelength " << *this << " in " << numTopModules();
	if (m_numNonTrivialTopModules != numTopModules())
		Log() << " (" << m_numNonTrivialTopModules << " non-trivial)";
	Log() << " modules." << std::endl;

	// Set consolidated cluster index on nodes and modules
	for (auto clusterIt = m_root.begin_tree(); !clusterIt.isEnd(); ++clusterIt) {
		clusterIt->index = clusterIt.clusterIndex();
	}

	// Calculate individual module codelengths and store on the modules
	calcCodelengthOnTree(false);
	m_root.codelength = getIndexCodelength();
	m_hierarchicalCodelength = getCodelength();
}


void InfomapBase::restoreHardPartition()
{
	// Collect all leaf nodes in a separate sequence to be able to iterate independent of tree structure
	std::vector<InfoNode*> leafNodes(numLeafNodes());
	unsigned int leafIndex = 0;
	for (InfoNode& node : m_root.infomapTree()) {
		if (node.isLeaf()) {
			leafNodes[leafIndex] = &node;
			++leafIndex;
		}
	}
	// Expand all children
	unsigned int numExpandedChildren = 0;
	unsigned int numExpandedNodes = 0;
	for (InfoNode* node : leafNodes) {
		++numExpandedNodes;
		numExpandedChildren += node->expandChildren();
		node->replaceWithChildren();
	}

	// Swap back original leaf nodes
	m_originalLeafNodes.swap(m_leafNodes);

	Log(1) << "Expanded " << numExpandedNodes << " hard modules to " << numExpandedChildren << " original nodes." << std::endl;
}

void InfomapBase::writeResult()
{
	if (this->noFileOutput)
		return;
	
	std::string outputFilename = this->outDirectory + this->outName + ".tree";
	Log() << "Write tree to " << outputFilename << "... ";

	SafeOutFile outFile(outputFilename);
	outFile << "# Codelength = " << getCodelength() << " bits.\n";
	// auto it = root.begin_treePath();
	// it++;
	// for (; !it.isEnd(); ++it) {
	// for (auto& it : root.infomapTree()) {
	for (auto it = root().begin_treePath(); !it.isEnd(); ++it) {
		InfoNode &node = *it;
		if (node.isLeaf()) {
			auto &path = it.path();
			outFile << io::stringify(path, ":", 1) << " " << node.data.flow << " \"" << node.uid << "\" " <<
					node.physicalId << "\n";
		}
	}
	
	Log() << "done!\n";
}

// ===================================================
// Run: Init: *
// ===================================================

void InfomapBase::initEnterExitFlow()
{
	// Calculate enter/exit
	for (auto* n : m_leafNodes) {
		n->data.enterFlow = n->data.exitFlow = 0.0;
	}
    if (this->directedEdges) {
        for (auto *n : m_leafNodes) {
            for (EdgeType *e : n->outEdges()) {
                EdgeType &edge = *e;
                edge.source.data.exitFlow += edge.data.flow;
                edge.target.data.enterFlow += edge.data.flow;
            }
        }
    }
    else {
        for (auto *n : m_leafNodes) {
            for (EdgeType *e : n->outEdges()) {
                EdgeType &edge = *e;
                double halfFlow = edge.data.flow / 2;
                // double halfFlow = edge.data.flow;
                edge.source.data.exitFlow += halfFlow;
                edge.target.data.exitFlow += halfFlow;
                edge.source.data.enterFlow += halfFlow;
                edge.target.data.enterFlow += halfFlow;
            }
        }
    }
}

double InfomapBase::calcCodelengthOnTree(bool includeRoot)
{
	double totalCodelength = 0.0;
	// Calculate enter/exit flow (assuming 0 by default)
	for (auto& node : m_root.infomapTree()) {
		if (node.isLeaf() || (!includeRoot && node.isRoot()))
			continue;
		node.codelength = calcCodelength(node);
		totalCodelength += node.codelength;
	}
	return totalCodelength;
}


// ===================================================
// Run: Partition: *
// ===================================================

void InfomapBase::setActiveNetworkFromLeafs()
{
	m_activeNetwork = &m_leafNodes;
}

void InfomapBase::setActiveNetworkFromChildrenOfRoot()
{
	m_moduleNodes.resize(m_root.childDegree());
	unsigned int i = 0;
	for (auto& node : m_root)
		m_moduleNodes[i++] = &node;
	m_activeNetwork = &m_moduleNodes;
}

void InfomapBase::findTopModulesRepeatedly(unsigned int maxLevels)
{
	Log(1,2) << "Iteration " << (m_tuneIterationIndex + 1) << ", moving ";
	Log(3) << "\nIteration " << (m_tuneIterationIndex + 1) << ":\n";
	m_aggregationLevel = 0;
	unsigned int numLevelsConsolidated = numLevels() - 1;
	if (maxLevels == 0)
		maxLevels = std::numeric_limits<unsigned int>::max();
	
	std::string initialCodelength;

	// Stopwatch timerAll(true);
	// Stopwatch timer(false);
	// Reapply core algorithm on modular network, replacing modules with super modules
	while (numTopModules() > 1 && numLevelsConsolidated != maxLevels)
	{
		// timer.start();
		if (haveModules())
			setActiveNetworkFromChildrenOfRoot();
		else
			setActiveNetworkFromLeafs();
		initPartition();
		if (m_aggregationLevel == 0) {
			initialCodelength = io::Str() << "" << *this;
		}

		Log(1,2) << activeNetwork().size() << "*" << std::flush;
		Log(3) << "Level " << (numLevelsConsolidated+1) << " (codelength: " << *this << "): Moving " <<
						activeNetwork().size() << " nodes... " << std::flush;
		// Log() << "{{ INIT TIME: " << timer.getElapsedTimeInMilliSec() << "ms }} ";
		// timer.start();
		// Core loop, merging modules
		unsigned int numOptimizationLoops = optimizeActiveNetwork();

		Log(1,2) << numOptimizationLoops << ", " << std::flush;
		Log(3) << "done! -> codelength " << *this << " in " << numActiveModules() << " modules." << std::endl;
		// Log() << "{{ MOVE TIME: " << timer.getElapsedTimeInMilliSec() << "ms }} ";

		// If no improvement, revert codelength terms to the last consolidated state
		if (haveModules() && restoreConsolidatedOptimizationPointIfNoImprovement())
		{
			Log(3) << "-> Restored to codelength " << *this << " in " << numTopModules() << " modules." << std::endl;
			break;
		}
		// timer.start();
		// Consolidate modules
		bool replaceExistingModules = haveModules();
		consolidateModules(replaceExistingModules);
		// Log() << "{{ CONSOLIDATE TIME: " << timer.getElapsedTimeInMilliSec() << "ms }} ";
		++numLevelsConsolidated;
		++m_aggregationLevel;
	}
	m_aggregationLevel = 0;

	calculateNumNonTrivialTopModules();

	Log(1,2) << (m_isCoarseTune ? "modules" : "nodes") << "*loops from codelength " <<
		initialCodelength << " to codelength " << *this <<
		" in " << numTopModules() << " modules. (" << m_numNonTrivialTopModules <<
		" non-trivial modules)" << std::endl;
	// Log() << "{{ TIME: " << timerAll.getElapsedTimeInMilliSec() << "ms }}\n";
}

unsigned int InfomapBase::fineTune()
{
	if (numLevels() != 2)
		throw InternalOrderError("InfomapBase::fineTune() called but numLevels != 2");

	setActiveNetworkFromLeafs();
	initPartition();

	// Make sure module nodes have correct index //TODO: Assume always correct here?
	unsigned int moduleIndex = 0;
	for (auto& module : m_root) {
		module.index = moduleIndex++;
	}

	// Put leaf modules in existing modules
	std::vector<unsigned int> modules(numLeafNodes());
	for (unsigned int i = 0; i < numLeafNodes(); ++i) {
		modules[i] = m_leafNodes[i]->parent->index;
	}
	moveActiveNodesToPredifinedModules(modules);

	Log(3) << " -> moved to codelength " << *this << " in " << numActiveModules() << " existing modules. Try tuning..." << std::endl;

	// Continue to optimize from there to tune leaf nodes
	unsigned int numEffectiveLoops = optimizeActiveNetwork();
	if (numEffectiveLoops == 0) {
		restoreConsolidatedOptimizationPointIfNoImprovement();
		Log(4) << "Fine-tune didn't improve solution, restoring last.\n";
	}
	else {
		// Delete existing modules and consolidate fine-tuned modules
		root().replaceChildrenWithGrandChildren();
		consolidateModules(false);
		Log(4) << "Fine-tune done in " << numEffectiveLoops << " effective loops to codelength " << *this <<
				" in " << numTopModules() << " modules." << std::endl;

	}

	return numEffectiveLoops;
}

unsigned int InfomapBase::coarseTune()
{
	if (numLevels() != 2)
		throw InternalOrderError("InfomapBase::coarseTune() called but numLevels != 2");

	Log(4) << "Coarse-tune...\nPartition each module in sub-modules for coarse tune..." << std::endl;

	bool isSilent = false;
	if (isMainInfomap()) {
		isSilent = Log::isSilent();
		Log::setSilent(true);
	}

	unsigned int moduleIndexOffset = 0;
	for (auto& node : m_root)
	{
		// Don't search for sub-modules in too small modules
		if (node.childDegree() < 2) {
			for (auto& child : node) {
				child.index = moduleIndexOffset;
			}
			moduleIndexOffset += 1;
			continue;
		}
		else {

			InfomapBase& subInfomap = getSubInfomap(node)
				.setTwoLevel(true)
				.setNoCoarseTune(true);
			subInfomap.initNetwork(node).run();

			auto originalLeafIt = node.begin_child();
			for (auto& subLeafPtr : subInfomap.leafNodes()) {
				InfoNode& subLeaf = *subLeafPtr;
				originalLeafIt->index = subLeaf.index + moduleIndexOffset;
				++originalLeafIt;
			}
			moduleIndexOffset += subInfomap.numTopModules();

			node.disposeInfomap();
		}
	}

	if (isMainInfomap())
		Log::setSilent(isSilent);

	Log(4) << "Move leaf nodes to " << moduleIndexOffset << " sub-modules... " << std::endl;
	// Put leaf modules in the calculated sub-modules
	std::vector<unsigned int> subModules(numLeafNodes());
	for (unsigned int i = 0; i < numLeafNodes(); ++i) {
		subModules[i] = m_leafNodes[i]->index;
	}

	setActiveNetworkFromLeafs();
	initPartition();
	moveActiveNodesToPredifinedModules(subModules);

	Log(4) << "Moved to " << numActiveModules() << " modules..." << std::endl;

	// Replace existing modules but store that structure on the sub-modules
	consolidateModules(true);

	Log(4) << "Consolidated " << numTopModules() << " sub-modules to codelength " << *this << std::endl;

	Log(4) << "Move sub-modules to former modular structure... " << std::endl;
	// Put sub-modules in the former modular structure
	std::vector<unsigned int> modules(numTopModules());
	unsigned int iModule = 0;
	for (auto& subModule : m_root) {
		modules[iModule++] = subModule.index;
	}

	setActiveNetworkFromChildrenOfRoot();
	initPartition();
	// Move sub-modules to former modular structure
	moveActiveNodesToPredifinedModules(modules);

	Log(4) << "Tune sub-modules from codelength " << *this << " in " << numActiveModules() << " modules... " << std::endl;
	// Continue to optimize from there to tune sub-modules
	unsigned int numEffectiveLoops = optimizeActiveNetwork();
	Log(4) << "Tuned sub-modules in " << numEffectiveLoops << " effective loops to codelength " << *this <<
			" in " << numActiveModules() << " modules." << std::endl;
	consolidateModules(true);
	Log(4) << "Consolidated tuned sub-modules to codelength " << *this << " in " << numTopModules() << " modules." << std::endl;
	return numEffectiveLoops;
}

void InfomapBase::calculateNumNonTrivialTopModules()
{
	m_numNonTrivialTopModules = 0;
	for (auto& module : m_root) {
		if (module.childDegree() != 1)
			++m_numNonTrivialTopModules;
	}
}

unsigned int InfomapBase::findHierarchicalSuperModulesFast(unsigned int superLevelLimit)
{
	if (superLevelLimit == 0)
		return 0;

	unsigned int numLevelsCreated = 0;
	double oldIndexLength = getIndexCodelength();
	double hierarchicalCodelength = getCodelength();
	double workingHierarchicalCodelength = hierarchicalCodelength;

	Log(1) << "\nFind hierarchical super modules iteratively..." << std::endl;
	Log(0,0) << "Fast super-level compression: " << std::setprecision(2) << std::flush;

	// Add index codebooks as long as the code gets shorter
	do
	{
		Log(1) << "Iteration " << numLevelsCreated + 1 << ", finding super modules fast to " << numTopModules() <<
				(haveModules()? " modules" : " nodes") << "... " << std::endl;

		if (haveModules())
		{
			setActiveNetworkFromChildrenOfRoot();
			transformNodeFlowToEnterFlow(m_root);
			initSuperNetwork();
		}
		else
		{
			setActiveNetworkFromLeafs();
		}

		initPartition();

		unsigned int numEffectiveLoops = optimizeActiveNetwork();

		double codelength = getCodelength();
		double indexCodelength = getIndexCodelength();
		unsigned int numSuperModules = numActiveModules();
		bool trivialSolution = numSuperModules == 1 || numSuperModules == activeNetwork().size();

		bool acceptSolution = !trivialSolution && codelength < oldIndexLength - this->minimumCodelengthImprovement;
		// Force at least one modular level!
		bool acceptByForce = !acceptSolution && !haveModules();
		if (acceptByForce)
			acceptSolution = true;

		workingHierarchicalCodelength += codelength - oldIndexLength;

		Log(0,0) << hideIf(!acceptSolution) << ((hierarchicalCodelength - workingHierarchicalCodelength) \
				/ hierarchicalCodelength * 100) << "% " << std::flush;
		Log(1) << "  -> Found " << numActiveModules() << " super modules in " << numEffectiveLoops <<
					" effective loops with hierarchical codelength " << indexCodelength << " + " <<
					(workingHierarchicalCodelength - indexCodelength) << " = " <<
					workingHierarchicalCodelength << (acceptSolution ? "\n" :
							", discarding the solution.\n") << std::flush;
		Log(1) << oldIndexLength << " -> " << *this << "\n";

		if (!acceptSolution)
		{
			restoreConsolidatedOptimizationPointIfNoImprovement(true);
			break;
		}

		// Consolidate the dynamic modules without replacing any existing ones.
		consolidateModules(false);

		hierarchicalCodelength = workingHierarchicalCodelength;
		oldIndexLength = indexCodelength;

		++numLevelsCreated;

		m_numNonTrivialTopModules = 0;
		for (auto& module : m_root) {
			if (module.childDegree() != 1)
				++m_numNonTrivialTopModules;
		}

	} while (m_numNonTrivialTopModules > 1 && numLevelsCreated != superLevelLimit);

	// Restore the temporary transformation of flow to enter flow on super modules
	resetFlowOnModules();

	Log(0,0) << "to codelength " << io::toPrecision(hierarchicalCodelength) << " in " <<
				numTopModules() << " top modules." << std::endl;
	Log(1) << "Finding super modules done! Added " << numLevelsCreated << " levels with hierarchical codelength " <<
			io::toPrecision(hierarchicalCodelength) << " in " << numTopModules() << " top modules." << std::endl;

	// Calculate and store hierarchical codelengths
	calcCodelengthOnTree();
	m_hierarchicalCodelength = hierarchicalCodelength;

	return numLevelsCreated;
}

unsigned int InfomapBase::findHierarchicalSuperModules(unsigned int superLevelLimit)
{
	if (superLevelLimit == 0)
		return 0;

	unsigned int numLevelsCreated = 0;
	double oldIndexLength = getIndexCodelength();
	double hierarchicalCodelength = getCodelength();
	double workingHierarchicalCodelength = hierarchicalCodelength;

	if (!haveModules())
		throw InternalOrderError("Trying to find hierarchical super modules without any modules");

	Log(1) << "\nFind hierarchical super modules iteratively..." << std::endl;
	Log(0,0) << "Super-level compression: " << std::setprecision(2) << std::flush;

	// Add index codebooks as long as the code gets shorter
	do
	{
		Log(1) << "Iteration " << numLevelsCreated + 1 << ", finding super modules to " << numTopModules() <<
				" modules... " << std::endl;
//		Log(1) << "\nTry indexing top modules with separate Infomap..." << std::endl;
		bool isSilent = false;
		if (isMainInfomap()) {
			isSilent = Log::isSilent();
			Log::setSilent(true);
		}
		InfoNode tmp(m_root.data); // Temporary owner for the super Infomap instance
		auto& superInfomap = getSuperInfomap(tmp)
			.setTwoLevel(true);
		superInfomap.initNetwork(m_root, true);//.initSuperNetwork();
		superInfomap.run();
		if (isMainInfomap()) {
			Log::setSilent(isSilent);
		}
//		Log(1) << "============= Super infomap done with codelength " << superInfomap << "\n";
		double superCodelength = superInfomap.getCodelength();
		double superIndexCodelength = superInfomap.getIndexCodelength();

		unsigned int numSuperModules = superInfomap.numTopModules();
		bool trivialSolution = numSuperModules == 1 || numSuperModules == numTopModules();

		if (trivialSolution) {
			Log(1) << "failed to find non-trivial super modules." << std::endl;
			break;
		}

		bool improvedCodelength = superCodelength < oldIndexLength - this->minimumCodelengthImprovement;
		if (!improvedCodelength) {
			Log(1) << "two-level index codebook not improved over one-level." << std::endl;
			break;
		}

		workingHierarchicalCodelength += superCodelength - oldIndexLength;

		Log(0,0) << ((hierarchicalCodelength - workingHierarchicalCodelength) \
				/ hierarchicalCodelength * 100) << "% " << std::flush;
		Log(1) << "  -> Found " << numSuperModules << " super modules in with hierarchical codelength " <<
				superIndexCodelength << " + " << (superCodelength - superIndexCodelength) << " + " <<
					(workingHierarchicalCodelength - superCodelength) << " = " <<
					workingHierarchicalCodelength << std::endl;

		// Merge current top modules to two-level solution found in the super Infomap instance.
		std::vector<unsigned int> modules(numTopModules());
		unsigned int iModule = 0;
		for (InfoNode* n : superInfomap.leafNodes()) {
			modules[iModule++] = n->index;
		}

		setActiveNetworkFromChildrenOfRoot();
		initPartition();
		// Move top modules to super modules
		moveActiveNodesToPredifinedModules(modules);

		// Consolidate the dynamic modules without replacing any existing ones.
		consolidateModules(false);

		Log(1) << "Consolidated " << numTopModules() << " super modules. Index codelength: " << oldIndexLength << " -> " << *this << "\n";

		hierarchicalCodelength = workingHierarchicalCodelength;
		oldIndexLength = superIndexCodelength;

		++numLevelsCreated;

		m_numNonTrivialTopModules = 0;
		for (auto& module : m_root) {
			if (module.childDegree() != 1)
				++m_numNonTrivialTopModules;
		}

	} while (m_numNonTrivialTopModules > 1 && numLevelsCreated != superLevelLimit);

	// Restore the temporary transformation of flow to enter flow on super modules
	resetFlowOnModules();

	Log(0,0) << "to codelength " << io::toPrecision(hierarchicalCodelength) << " in " <<
				numTopModules() << " top modules." << std::endl;
	Log(1) << "Finding super modules done! Added " << numLevelsCreated << " levels with hierarchical codelength " <<
			io::toPrecision(hierarchicalCodelength) << " in " << numTopModules() << " top modules." << std::endl;

	// Calculate and store hierarchical codelengths
	m_hierarchicalCodelength = hierarchicalCodelength;

	return numLevelsCreated;


}


void InfomapBase::transformNodeFlowToEnterFlow(InfoNode& parent)
{
	for (auto& module : m_root)
	{
		module.data.flow = module.data.enterFlow;
	}
}

void InfomapBase::resetFlowOnModules()
{
	// Reset flow on all modules
	for (auto& module : m_root.infomapTree()) {
		if (!module.isLeaf())
			module.data.flow = 0.0;
	}
	// Aggregate flow from leaf nodes up in the tree
	for (InfoNode* n : m_leafNodes) {
		double leafNodeFlow = n->data.flow;
		InfoNode* module = n->parent;
		do {
			module->data.flow += leafNodeFlow;
			module = module->parent;
		} while (module != nullptr);
	}
}

unsigned int InfomapBase::removeModules()
{
	unsigned int numLevelsDeleted = 0;
	while (numLevels() > 1) {
		m_root.replaceChildrenWithGrandChildren();
		++numLevelsDeleted;
	}
	if (numLevelsDeleted > 0) {
		Log(2) << "Removed " << numLevelsDeleted << " levels of modules." << std::endl;
	}
	return numLevelsDeleted;
}

unsigned int InfomapBase::removeSubModules(bool recalculateCodelengthOnTree)
{
	unsigned int numLevelsDeleted = 0;
	while (numLevels() > 2) {
		for (InfoNode& module : m_root)
			module.replaceChildrenWithGrandChildren();
		++numLevelsDeleted;
	}
	if (numLevelsDeleted > 0) {
		Log(2) << "Removed " << numLevelsDeleted << " levels of sub-modules." << std::endl;
		if (recalculateCodelengthOnTree)
			calcCodelengthOnTree(false);
	}
	return numLevelsDeleted;
}


unsigned int InfomapBase::recursivePartition()
{
	double indexCodelength = getIndexCodelength();
	double hierarchicalCodelength = m_hierarchicalCodelength;

	calcCodelengthOnTree(true);

	Log(0,0) << "\nRecursive sub-structure compression: " << std::flush;

	PartitionQueue partitionQueue;
	if (this->fastHierarchicalSolution > 0) {
		queueLeafModules(partitionQueue);
		Log(1) << "\nFind sub modules recursively from " << partitionQueue.size() << " sub modules on level " << numLevels() - 2;
	}
	else {
		queueTopModules(partitionQueue);
		Log(1) << "\nFind sub modules recursively from " << partitionQueue.size() << " top modules";
		double moduleCodelength = 0.0;
		for (auto& module : m_root)
			moduleCodelength += module.codelength;
		hierarchicalCodelength = indexCodelength + moduleCodelength;
	}
	Log(1) << " with codelength: " << indexCodelength << " + " <<
		(hierarchicalCodelength - indexCodelength) << " = " <<
		io::toPrecision(hierarchicalCodelength) << std::endl;

	double sumConsolidatedCodelength = hierarchicalCodelength - partitionQueue.moduleCodelength;

	bool isSilent = false;
	if (isMainInfomap()) {
		isSilent = Log::isSilent();
	}

	while (partitionQueue.size() > 0)
	{
		Log(1) << "Level " << partitionQueue.level << ": " << (partitionQueue.flow*100) <<
				"% of the flow in " << partitionQueue.size() << " modules. Partitioning... " <<
				std::setprecision(6) << std::flush;

		if (isMainInfomap())
			Log::setSilent(true);

		// Partition all modules in the queue and fill up the next level queue
		PartitionQueue nextLevelQueue;
		processPartitionQueue(partitionQueue, nextLevelQueue);

		if (isMainInfomap())
			Log::setSilent(isSilent);

		double leftToImprove = partitionQueue.moduleCodelength;
		sumConsolidatedCodelength += partitionQueue.indexCodelength + partitionQueue.leafCodelength;
		double limitCodelength = sumConsolidatedCodelength + leftToImprove;

		Log(0,0) << ((hierarchicalCodelength - limitCodelength) / hierarchicalCodelength) * 100 <<
			"% " << std::flush;
		Log(1) << "done! Codelength: " << partitionQueue.indexCodelength << " + " <<
					partitionQueue.leafCodelength << " (+ " << leftToImprove << " left to improve)" <<
					" -> limit: " << io::toPrecision(limitCodelength) << " bits.\n";

		hierarchicalCodelength = limitCodelength;

		partitionQueue.swap(nextLevelQueue);
	}

	// Store resulting hierarchical codelength
	m_hierarchicalCodelength = hierarchicalCodelength;

	Log(0,0) << ". Found " << partitionQueue.level << " levels with codelength " <<
		io::toPrecision(hierarchicalCodelength) << "\n";
	Log(1) << "  -> Found " << partitionQueue.level << " levels with codelength " <<
		io::toPrecision(hierarchicalCodelength) << "\n";

	return partitionQueue.level;
}

void InfomapBase::queueTopModules(PartitionQueue& partitionQueue)
{
//	Log(3) << "Queue " << numTopModules() << " top modules..." << std::endl;
	// Add modules to partition queue
	unsigned int numNonTrivialModules = 0;
	partitionQueue.resize(numTopModules());
	double sumFlow = 0.0;
	double sumNonTrivialFlow = 0.0;
	double sumModuleCodelength = 0.0;
	unsigned int moduleIndex = 0;
	for (auto& module : m_root)
	{
		partitionQueue[moduleIndex] = &module;
		sumFlow += module.data.flow;
		sumModuleCodelength += module.codelength;
		if (module.childDegree() > 1)
		{
			++numNonTrivialModules;
			sumNonTrivialFlow += module.data.flow;
		}
		++moduleIndex;
//		Log(3) << "  " << moduleIndex << ": Flow: " << module.data.flow << ", codelength: " << module.codelength << ", childDegree: " << module.childDegree() << std::endl;
	}
	partitionQueue.flow = sumFlow;
	partitionQueue.numNonTrivialModules = numNonTrivialModules;
	partitionQueue.nonTrivialFlow = sumNonTrivialFlow;
	partitionQueue.indexCodelength = getIndexCodelength();
	partitionQueue.moduleCodelength = sumModuleCodelength;
	partitionQueue.level = 1;
}

void InfomapBase::queueLeafModules(PartitionQueue& partitionQueue)
{
	unsigned int numLeafModules = 0;
	for (auto& node : m_root.infomapTree()) {
		if (node.isLeafModule())
			++numLeafModules;
	}

//	Log(3) << "Queue " << numLeafModules << " leaf modules..." << std::endl;

	// Add modules to partition queue
	partitionQueue.resize(numLeafModules);
	unsigned int numNonTrivialModules = 0;
	double sumFlow = 0.0;
	double sumNonTrivialFlow = 0.0;
	double sumModuleCodelength = 0.0;
	unsigned int moduleIndex = 0;
	unsigned int maxDepth = 0;
	for (auto it = m_root.begin_tree(); !it.isEnd(); ++it) {
		if (it->isLeafModule()) {
			auto& module = *it;
			partitionQueue[moduleIndex] = it.base();
			sumFlow += module.data.flow;
			sumModuleCodelength += module.codelength;
			if (module.childDegree() > 1)
			{
				++numNonTrivialModules;
				sumNonTrivialFlow += module.data.flow;
			}
			maxDepth = std::max(maxDepth, it.depth());
			++moduleIndex;
//			Log(3) << "  " << moduleIndex << ": Flow: " << module.data.flow << ", codelength: " << module.codelength << ", childDegree: " << module.childDegree() << std::endl;
		}
	}
	partitionQueue.flow = sumFlow;
	partitionQueue.numNonTrivialModules = numNonTrivialModules;
	partitionQueue.nonTrivialFlow = sumNonTrivialFlow;
	partitionQueue.indexCodelength = getIndexCodelength();
	partitionQueue.moduleCodelength = sumModuleCodelength;
	partitionQueue.level = maxDepth;
}

bool InfomapBase::processPartitionQueue(PartitionQueue& queue, PartitionQueue& nextLevelQueue)
{
	PartitionQueue::size_t numModules = queue.size();
	std::vector<double> indexCodelengths(numModules, 0.0);
	std::vector<double> moduleCodelengths(numModules, 0.0);
	std::vector<double> leafCodelengths(numModules, 0.0);
	std::vector<PartitionQueue> subQueues(numModules);

#pragma omp parallel for schedule(dynamic)
	for(PartitionQueue::size_t moduleIndex = 0; moduleIndex < numModules; ++moduleIndex)
	{
		InfoNode& module = *queue[moduleIndex];

		// Delete former sub-structure if exists
		if (module.disposeInfomap())
			module.codelength = calcCodelength(module);

		// If only trivial substructure is to be found, no need to create infomap instance to find sub-module structures.
		if (module.childDegree() <= 2)
		{
			module.codelength = calcCodelength(module);
			leafCodelengths[moduleIndex] = module.codelength;
			continue;
		}

		double oldModuleCodelength = module.codelength;
		PartitionQueue& subQueue = subQueues[moduleIndex];
		subQueue.level = queue.level + 1;

//		std::cout << "\n\n\nSubInfomap partition network with " << module.childDegree() << " nodes (module codelength: " <<
//				module.codelength << ")...:\n" << std::endl;

		auto& subInfomap = getSubInfomap(module)
			.initNetwork(module);
		// Run two-level partition + find hierarchically super modules (skip recursion)
		subInfomap.setOnlySuperModules(true).run();
//		subInfomap.setTwoLevel(true).run();

		double subCodelength = subInfomap.getHierarchicalCodelength();
		double subIndexCodelength = subInfomap.getIndexCodelength();
		double subModuleCodelength = subCodelength - subIndexCodelength;
		InfoNode& subRoot = *module.getInfomapRoot();
		unsigned int numSubModules = subRoot.childDegree();
		bool trivialSubPartition = numSubModules == 1 || numSubModules == module.childDegree();
//		std::cout << "\nSubInfomap with " << module.childDegree() << " nodes (module codelength: " <<
//				module.codelength << ") partitioned to hierarchical codelength " <<
//				subInfomap.getHierarchicalCodelength() << " (" << subInfomap << ") in " <<
//				subInfomap.numTopModules() << " (" << numSubModules << ") top modules." << std::endl;
		bool improvedCodelength = subCodelength < oldModuleCodelength - this->minimumCodelengthImprovement;

		if (trivialSubPartition || !improvedCodelength) {
			Log(1) << "Disposing unaccepted sub Infomap instance." << std::endl;
			module.disposeInfomap();
			module.codelength = oldModuleCodelength;
			subQueue.skip = true;
			leafCodelengths[moduleIndex] = module.codelength;
		}
		else {
			// Improvement
			subInfomap.queueTopModules(subQueue);
			indexCodelengths[moduleIndex] = subIndexCodelength;
			moduleCodelengths[moduleIndex] = subModuleCodelength;
		}
	}

	double sumLeafCodelength = 0.0;
	double sumIndexCodelength = 0.0;
	double sumModuleCodelengths = 0.0;
	PartitionQueue::size_t nextLevelSize = 0;
	for (PartitionQueue::size_t moduleIndex = 0; moduleIndex < numModules; ++moduleIndex)
	{
		nextLevelSize += subQueues[moduleIndex].skip ? 0 : subQueues[moduleIndex].size();
		sumLeafCodelength += leafCodelengths[moduleIndex];
		sumIndexCodelength += indexCodelengths[moduleIndex];
		sumModuleCodelengths += moduleCodelengths[moduleIndex];
	}

	queue.indexCodelength = sumIndexCodelength;
	queue.leafCodelength = sumLeafCodelength;
	queue.moduleCodelength = sumModuleCodelengths;

	// Collect the sub-queues and build the next-level queue
	nextLevelQueue.level = queue.level + 1;
	nextLevelQueue.resize(nextLevelSize);
	PartitionQueue::size_t nextLevelIndex = 0;
	for (PartitionQueue::size_t moduleIndex = 0; moduleIndex < numModules; ++moduleIndex)
	{
		PartitionQueue& subQueue = subQueues[moduleIndex];
		if (!subQueue.skip)
		{
			for (PartitionQueue::size_t subIndex = 0; subIndex < subQueue.size(); ++subIndex)
			{
				nextLevelQueue[nextLevelIndex++] = subQueue[subIndex];
			}
			nextLevelQueue.flow += subQueue.flow;
			nextLevelQueue.nonTrivialFlow += subQueue.nonTrivialFlow;
			nextLevelQueue.numNonTrivialModules += subQueue.numNonTrivialModules;
		}
	}

	return nextLevelSize > 0;
}



unsigned int InfomapBase::printPerLevelCodelength(std::ostream& out)
{
	std::vector<PerLevelStat> perLevelStats;
	aggregatePerLevelCodelength(perLevelStats);

	unsigned int numLevels = perLevelStats.size();
	double averageNumNodesPerLevel = 0.0;
	for (unsigned int i = 0; i < numLevels; ++i)
		averageNumNodesPerLevel += perLevelStats[i].numNodes();
	averageNumNodesPerLevel /= numLevels;

	out << "Per level number of modules:         [";
	for (unsigned int i = 0; i < numLevels - 1; ++i)
	{
		out << io::padValue(perLevelStats[i].numModules, 11) << ", ";
	}
	out << io::padValue(perLevelStats[numLevels-1].numModules, 11) << "]";
	unsigned int sumNumModules = 0;
	for (unsigned int i = 0; i < numLevels; ++i)
		sumNumModules += perLevelStats[i].numModules;
	out << " (sum: " << sumNumModules << ")" << std::endl;

	out << "Per level number of leaf nodes:      [";
	for (unsigned int i = 0; i < numLevels - 1; ++i)
	{
		out << io::padValue(perLevelStats[i].numLeafNodes, 11) << ", ";
	}
	out << io::padValue(perLevelStats[numLevels-1].numLeafNodes, 11) << "]";
	unsigned int sumNumLeafNodes = 0;
	for (unsigned int i = 0; i < numLevels; ++i)
		sumNumLeafNodes += perLevelStats[i].numLeafNodes;
	out << " (sum: " << sumNumLeafNodes << ")" << std::endl;


	out << "Per level average child degree:      [";
	double childDegree = perLevelStats[0].numNodes();
	double sumAverageChildDegree = childDegree * childDegree;
	if (numLevels > 1) {
		out << io::padValue(perLevelStats[0].numModules, 11) << ", ";
	}
	for (unsigned int i = 1; i < numLevels - 1; ++i)
	{
		childDegree = perLevelStats[i].numNodes() * 1.0 / perLevelStats[i-1].numModules;
		sumAverageChildDegree += childDegree * perLevelStats[i].numNodes();
		out << io::padValue(childDegree, 11) << ", ";
	}
	if (numLevels > 1) {
		childDegree = perLevelStats[numLevels-1].numNodes() * 1.0 / perLevelStats[numLevels-2].numModules;
		sumAverageChildDegree += childDegree * perLevelStats[numLevels-1].numNodes();
	}
	out << io::padValue(childDegree, 11) << "]";
	out << " (average: " << sumAverageChildDegree/(sumNumModules + sumNumLeafNodes) << ")" << std::endl;

	out << std::fixed << std::setprecision(9);
	out << "Per level codelength for modules:    [";
	for (unsigned int i = 0; i < numLevels - 1; ++i)
	{
		out << perLevelStats[i].indexLength << ", ";
	}
	out << perLevelStats[numLevels-1].indexLength << "]";
	double sumIndexLengths = 0.0;
	for (unsigned int i = 0; i < numLevels; ++i)
		sumIndexLengths += perLevelStats[i].indexLength;
	out << " (sum: " << sumIndexLengths << ")" << std::endl;

	out << "Per level codelength for leaf nodes: [";
	for (unsigned int i = 0; i < numLevels - 1; ++i)
	{
		out << perLevelStats[i].leafLength << ", ";
	}
	out << perLevelStats[numLevels-1].leafLength << "]";

	double sumLeafLengths = 0.0;
	for (unsigned int i = 0; i < numLevels; ++i)
		sumLeafLengths += perLevelStats[i].leafLength;
	out << " (sum: " << sumLeafLengths << ")" << std::endl;

	out << "Per level codelength total:          [";
	for (unsigned int i = 0; i < numLevels - 1; ++i)
	{
		out << perLevelStats[i].codelength() << ", ";
	}
	out << perLevelStats[numLevels-1].codelength() << "]";

	double sumCodelengths = 0.0;
	for (unsigned int i = 0; i < numLevels; ++i)
		sumCodelengths += perLevelStats[i].codelength();
	out << " (sum: " << sumCodelengths << ")" << std::endl;

	return numLevels;
}

void InfomapBase::aggregatePerLevelCodelength(std::vector<PerLevelStat>& perLevelStat, unsigned int level)
{
	aggregatePerLevelCodelength(root(), perLevelStat, level);
}

void InfomapBase::aggregatePerLevelCodelength(InfoNode& parent, std::vector<PerLevelStat>& perLevelStat, unsigned int level)
{
	if (perLevelStat.size() < level+1)
		perLevelStat.resize(level+1);
	
	if (parent.firstChild->isLeaf()) {
		perLevelStat[level].numLeafNodes += parent.childDegree();
		perLevelStat[level].leafLength += parent.codelength;
		return;
	}

	perLevelStat[level].numModules += parent.childDegree();
	perLevelStat[level].indexLength += parent.isRoot() ? getIndexCodelength() : parent.codelength;

	for (auto& module : parent)
	{
		if (module.getInfomapRoot() != nullptr)
			module.getInfomap().aggregatePerLevelCodelength(perLevelStat, level+1);
		else
			aggregatePerLevelCodelength(module, perLevelStat, level+1);
	}
}

}
