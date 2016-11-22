#pragma once

#include "../Simulation.h"

#include<functional>
#include<unordered_map>
#include<unordered_set>
#include<vector>

namespace sp {

	void FindMIS(const std::vector<std::vector<bool>>& graph, std::vector<int>& indices);

	void FindMISAlt(const std::vector<std::vector<bool>>& graph, std::vector<int>& indices);

	class Projection2D {
	public:
		Projection2D(int fx, int fy) : fx(fx), fy(fy) {}

		void FindSimpleMIS(const std::vector<Rule>& rl, std::vector<Rule>& mis, std::vector<Rule>& remain) const;
		void FindMIS(const std::vector<Rule>& rl, std::vector<Rule>& mis, std::vector<Rule>& remain) const;

		std::vector<const Rule*> MaxClique(const std::vector<const Rule*>& v) const;

		bool DoesIntersect(const Rule& r, const Rule& s) const;
		bool CornerIntersection(const Rule& r, const Rule& s) const;
		bool NonCornerIntersection(const Rule& r, const Rule& s) const;

		bool IsBadRectangle(const Rule& r, const std::vector<const Rule*>& rl, int thresh) const;

	private:
		void SimpleIteration(std::vector<const Rule*>& rl, std::unordered_map<const Rule*, int>& vr, std::vector<std::vector<const Rule*>>& tv, size_t q) const;
		void Iteration(std::vector<const Rule*>& rl, std::vector<std::vector<const Rule*>>& t, const std::unordered_map<const Rule*, int>& vr, int i, int beta, int m) const;
		int FindV(const std::vector<const Rule*>& v, const Rule& r) const;
		void RemoveBadRectangles(std::vector<std::vector<const Rule*>>& sv, int thresh) const;
		void RemoveLargeClique(std::vector<const Rule*>& rl, std::vector<const Rule*>& clique, std::vector<std::vector<const Rule*>>& sv, std::vector<std::vector<const Rule*>>& tv, size_t j) const;
		int CliqueHelper(const std::vector<const Rule*>& v, unsigned int y, unsigned int& x) const;
		std::vector<double> LPSolution(const std::vector<const Rule*>& rl) const;

		int fx, fy;
	};

	template<class T>
	std::vector<const T*> ToPointerVector(const std::vector<T>& v) {
		std::vector<const T*> w;
		w.reserve(v.size());
		for (const T& t : v) {
			const T* p = &t;
			w.push_back(p);
		}
		return w;
	}

	template<class T>
	std::vector<T> Union3(const std::vector<T>& v1, const std::vector<T>& v2, const std::vector<T>& v3) {
		std::unordered_set<T> hs(v1.begin(), v1.end());
		hs.insert(v2.begin(), v2.end());
		hs.insert(v3.begin(), v3.end());
		return std::vector<T>(hs.begin(), hs.end());
	}

	template<class S, class T>
	std::vector<T> ConvertDistinct(const std::vector<S>& v, std::function<T(S)> f) {
		std::unordered_set<T> w;
		for (const S& s : v) {
			w.insert(f(s));
		}
		return std::vector<T>(w.begin(), w.end());
		//r.assign(w.begin(), w.end());
	}

	template<class S, class T>
	std::vector<T> Convert(const std::vector<S>& v, std::function<T(S)> f) {
		std::vector<T> w;
		for (const S& s : v) {
			w.push_back(f(s));
		}
		return w;
		//r.assign(w.begin(), w.end());
	}

	template<class T>
	std::vector<T> Distinct(const std::vector<T>& v) {
		std::unordered_set<T> w;
		for (const T& x : v) {
			w.insert(x);
		}
		return std::vector<T>(w.begin(), w.end());
	}
}


