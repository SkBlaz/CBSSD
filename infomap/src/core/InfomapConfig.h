/*
 * InfomapConfig.h
 *
 *  Created on: 2 mar 2015
 *      Author: Daniel
 */

#ifndef MODULES_CLUSTERING_CLUSTERING_INFOMAPCONFIG_H_
#define MODULES_CLUSTERING_CLUSTERING_INFOMAPCONFIG_H_

#include "../io/Config.h"
#include <string>
#include "../utils/Random.h"
#include "../utils/Log.h"

namespace infomap {

template<typename Infomap>
class InfomapConfig : public Config {
public:
	InfomapConfig() {}
	InfomapConfig(const Config& conf) :
		Config(conf),
		m_rand(conf.seedToRandomNumberGenerator)
	{
		Log::precision(conf.verboseNumberPrecision);
	}
	virtual ~InfomapConfig() {}

private:
	Infomap& get() {
		return static_cast<Infomap&>(*this);
	}
protected:
	Random m_rand;

public:

	Config& getConfig() {
		return *this;
	}

	Infomap& setConfig(const Config& conf) {
		*this = conf;
		m_rand.seed(conf.seedToRandomNumberGenerator);
		Log::precision(conf.verboseNumberPrecision);
		return get();
	}

	Infomap& setNonMainConfig(const Config& conf) {
		cloneAsNonMain(conf);
		return get();
	}

	Infomap& setNumTrials(unsigned int N) {
		numTrials = N;
		return get();
	}

	Infomap& setTwoLevel(bool value) {
		twoLevel = value;
		return get();
	}

	Infomap& setFastHierarchicalSolution(unsigned int level) {
		fastHierarchicalSolution = level;
		return get();
	}

	Infomap& setOnlySuperModules(bool value) {
		onlySuperModules = value;
		return get();
	}

	Infomap& setNoCoarseTune(bool value) {
		noCoarseTune = value;
		return get();
	}

	Infomap& setDirected(bool directed) {
		directedEdges = directed;
		return get();
	}

	Infomap& setMarkovTime(double codeRate) {
		markovTime = codeRate;
		return get();
	}
};

}

#endif /* MODULES_CLUSTERING_CLUSTERING_INFOMAPCONFIG_H_ */
