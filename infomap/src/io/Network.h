/**********************************************************************************

 Infomap software package for multi-level network clustering

 Copyright (c) 2013, 2014 Daniel Edler, Martin Rosvall

 For more information, see <http://www.mapequation.org>


 This file is part of Infomap software package.

 Infomap software package is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Infomap software package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with Infomap software package.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************/


#ifndef NETWORK_H_
#define NETWORK_H_

#include <string>
#include <map>
#include <vector>
#include <set>
#include <utility>
#include "Config.h"
#include <limits>
#include <sstream>
#include "../core/StateNetwork.h"
#include <locale>

namespace infomap {

struct Bigram;
struct Weight;
struct BipartiteLink;

class Network : public StateNetwork
{
protected:

	// Helpers
	std::istringstream m_extractor;

	// Bipartite
	std::map<BipartiteLink, Weight> m_bipartiteLinks;
	unsigned int m_numBipartiteNodes;

	struct InsensitiveCompare {
		bool operator() (const std::string& a, const std::string& b) const {
			auto lhs = a.begin();
			auto rhs = b.begin();

			std::locale loc;
			for (; lhs != a.end() && rhs != b.end(); ++lhs,++rhs)
			{
				auto lhs_val = std::tolower(*lhs, loc);
				auto rhs_val = std::tolower(*rhs, loc);

				if (lhs_val != rhs_val)
					return lhs_val < rhs_val;
			}

			return (rhs != b.end());
		}
	};
	using InsensitiveStringSet = std::set<std::string, InsensitiveCompare>;

	std::map<std::string, InsensitiveStringSet> m_ignoreHeadings;
	std::map<std::string, InsensitiveStringSet> m_validHeadings;// {
	// 	{ "pajek", {"*Vertices", "*Edges", "*Arcs"} },
	// 	{ "link-list", {"*Links"} },
	// 	{ "bipartite", {"*Vertices", "*Bipartite"} },
	// 	{ "general", {"*Vertices", "*States", "*Edges", "*Arcs", "*Links", "*Context"} }
	// };

public:

	Network()
	:	StateNetwork() { initValidHeadings(); }
	Network(const Config& config)
	:	StateNetwork(config) { initValidHeadings(); }
	virtual ~Network() {}


	virtual void readInputData(std::string filename = "");

	// void printParsingResult(bool onlySummary = false);

	// std::string getParsingResultSummary();

protected:
	void initValidHeadings();

	void parsePajekNetwork(std::string filename);
	void parseLinkList(std::string filename);
	void parseStateNetwork(std::string filename);
	void parseNetwork(std::string filename);
	void parseNetwork(std::string filename, const InsensitiveStringSet& validHeadings, const InsensitiveStringSet& ignoreHeadings);
	// void parseNetwork(std::string filename, InsensitiveStringSet&& validHeadings = {
	// 	"*Vertices", "*States", "*Edges", "*Arcs", "*Links", "*Context"
	// });
	void parseBipartiteNetwork(std::string filename);

	// Helper methods

	/**
	 * Parse vertices under the heading
	 * @return The line after the vertices
	 */
	std::string parseVertices(std::ifstream& file, std::string heading);
	std::string parseStateNodes(std::ifstream& file, std::string heading);

	std::string parseLinks(std::ifstream& file);

	std::string parseBipartiteLinks(std::ifstream& file);

	std::string ignoreSection(std::ifstream& file, std::string heading);


	void parseStateNode(const std::string& line, StateNetwork::StateNode& stateNode);

	/**
	 * Parse a string of link data.
	 * If no weight data can be extracted, the default value 1.0 will be used.
	 * @throws an error if not both node numbers can be extracted.
	 */
	void parseLink(const std::string& line, unsigned int& n1, unsigned int& n2, double& weight);

	/**
	 * Parse a bipartite link of format "f1 n1 1.0" for a link between
	 * feature node 1 to ordinary node 1 with weight 1.0.
	 * The order of the feature nodes and ordinary nodes can be swapped.
	 * Store the numberical id (minus possible indexOffset for non-zerobased indexing)
	 * on the referenced uints.
	 * @return true if the input order was swapped
	 */
	bool parseBipartiteLink(const std::string& line, unsigned int& featureNode, unsigned int& node, double& weight);

	void printSummary();

};

struct Bigram
{
	unsigned int first, second;
	Bigram(unsigned int first = 0, unsigned int second = 0) : first(first), second(second) {}

	bool operator<(const Bigram other) const
	{
		return first == other.first ? second < other.second : first < other.first;
	}
};

struct BipartiteLink
{
	unsigned int featureNode, node;
	bool swapOrder;
	BipartiteLink(unsigned int featureNode = 0, unsigned int node = 0, bool swapOrder = false)
	: featureNode(featureNode), node(node), swapOrder(swapOrder) {}

	bool operator<(const BipartiteLink other) const
	{
		return featureNode == other.featureNode ? node < other.node : featureNode < other.featureNode;
	}
};

// Struct to make the weight initialized to zero by default in a map
struct Weight
{
	double weight;
	Weight(double weight = 0) : weight(weight) {}

	Weight& operator+=(double w)
	{
		weight += w;
		return *this;
	}
};

template<typename key_t, typename subkey_t, typename value_t>
class MapMap
{
public:
	typedef std::map<subkey_t, value_t> submap_t;
	typedef std::map<key_t, submap_t> map_t;
	MapMap() :
	m_size(0),
	m_numAggregations(0),
	m_sumValue(0)
	{}
	virtual ~MapMap() {}

	bool insert(key_t key1, subkey_t key2, value_t value)
	{
		++m_size;
		m_sumValue += value;

		// Aggregate link weights if they are definied more than once
		typename map_t::iterator firstIt = m_data.lower_bound(key1);
		if (firstIt != m_data.end() && firstIt->first == key1)
		{
			std::pair<typename submap_t::iterator, bool> ret2 = firstIt->second.insert(std::make_pair(key2, value));
			if (!ret2.second)
			{
				ret2.first->second += value;
				++m_numAggregations;
				--m_size;
				return false;
			}
		}
		else
		{
			m_data.insert(firstIt, std::make_pair(key1, submap_t()))->second.insert(std::make_pair(key2, value));
		}

		return true;
	}

	unsigned int size() { return m_size; }
	unsigned int numAggregations() { return m_numAggregations; }
	value_t sumValue() { return m_sumValue; }


private:
	map_t m_data;
	unsigned int m_size;
	unsigned int m_numAggregations;
	value_t m_sumValue;
};

typedef MapMap<unsigned int, unsigned int, double> LinkMapMap;

template<typename key_t, typename value_t>
class EasyMap : public std::map<key_t, value_t>
{
public:
	typedef std::map<key_t, value_t> map_t;
	typedef EasyMap<key_t, value_t> self_t;
	value_t& getOrSet(const key_t& key, value_t defaultValue = 0)
	{
		typename self_t::iterator it = this->lower_bound(key);
		if (it != this->end() && it->first == key)
			return it->second;
		return this->insert(it, std::make_pair(key, defaultValue))->second;
	}
};

struct Triple
{
	Triple() :
		n1(0), n2(0), n3(0) {}
	Triple(unsigned int value1, unsigned int value2, unsigned int value3) :
		n1(value1), n2(value2), n3(value3) {}
	Triple(const Triple& other) :
		n1(other.n1), n2(other.n2), n3(other.n3) {}
	~Triple() {}

	bool operator<(const Triple& other) const
	{
		return n1 == other.n1 ? (n2 == other.n2 ? n3 < other.n3 : n2 < other.n2) : n1 < other.n1;
	}

	bool operator==(const Triple& other) const
	{
		return n1 == other.n1 && n2 == other.n2 && n3 == other.n3;
	}

	unsigned int n1;
	unsigned int n2;
	unsigned int n3;
};

}

#endif /* NETWORK_H_ */
