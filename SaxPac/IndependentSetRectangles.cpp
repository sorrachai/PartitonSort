#include "IndependentSetRectangles.h"

#include "ClpSimplex.hpp"

#include<limits>

using namespace std;

namespace sp {

	bool InRange(unsigned int x, array<unsigned int, 2> range) {
		//printf("%u in [%u, %u]?\n", x, range[LowDim], range[HighDim]);
		return range[LowDim] <= x && x <= range[HighDim];
	}

	/**
	 * Finds the maximum independent set by means of the standard greedy algorithm (not the one used in the MISR paper)
	 * The vertices with minimum degree are added first until all remaining vertices are adjacent to at least one vertex in the independent set.
	 * Further ties are broken arbitrarily.
	 * Input: a 2d graph showing which pairs CANNOT end up together
	 * Output: a list of indices for the MIS
	*/
	void FindMIS(const vector<vector<bool>>& graph, vector<int>& mis) {
		vector<int> potentialSet;
		for (size_t i = 0; i < graph.size(); i++) {
			bool isSeparate = true;
			for (size_t j = 0; j < graph.size(); j++) {
				if (graph[i][j]) isSeparate = false;
			}
			if (isSeparate) {
				mis.push_back(i);
			} else {
				potentialSet.push_back(i);
			}
		}
		//printf("Enter: %u items\n", potentialSet.size());
		while (!potentialSet.empty()) {
			//for (int x : potentialSet) {
			//	printf("%d ", x);
			//}
			//printf("\n");
			//if (mis.size() % 100 == 0) {
			//	printf("MIS: %u, remain: %u\n", mis.size(), potentialSet.size());
			//}

			int minSize = numeric_limits<int>::max();
			int minI = -1;

			vector<int> empties;

			for (int i : potentialSet) {
				int size = 0;
				for (int j : potentialSet) {
					if (graph[i][j]) size++;
				}

				if (size == 0) {
					empties.push_back(i);
				}
				if (size < minSize) {
					minSize = size;
					minI = i;
				}
			}

			/*printf("minsize: %d\n", minSize);
			for (int j : potentialSet) {
				if (graph[minI][j]) {
					printf("%d vs %d\n", minI, j);
				}
			}*/
			//if (minSize == 0) {
			//	printf("disjoint!\n");
			//}
			
			if (empties.empty()) {
				mis.push_back(minI);
				potentialSet.erase(remove_if(potentialSet.begin(), potentialSet.end(), [=, &graph](int x) { return x == minI || graph[minI][x]; }), potentialSet.end());
			} else {
				//printf("Empty: %u items\n", empties.size());
				for (int i : empties) {
					mis.push_back(i);
				}
				size_t index = 0;
				vector<int> ps;
				for (int i : potentialSet) {
					if (index < empties.size() && i == empties[index]) {
						index++;
					} else {
						ps.push_back(i);
					}
				}
				potentialSet = ps;
			}

			
			//printf("Item:%d Cost:%d Remain:%u\n", minI, minSize, potentialSet.size());
			
		}
		//printf("Return: %u items\n", mis.size());
	}

	void FindMISAlt(const vector<vector<bool>>& graph, vector<int>& mis) {
		// Setup our objective
		//printf("MIS-Alt: %u rules\n", graph.size());
		ClpSimplex model;
		model.messageHandler()->setLogLevel(0); // No Messages
		model.resize(0, graph.size());
		for (size_t i = 0; i < graph.size(); i++) {
			model.setObjectiveCoefficient(i, -1); // Want to maximize, but CLP minimizes
			model.setColumnBounds(i, 0.0, 1.0);
		}

		// Add our constraints
		int* indices = new int[2];
		double* weights = new double[2]{ 1.0, 1.0 };
		int loopCount = 0;
		bool addedStuff = false;

		for (size_t i = 0; i < graph.size(); i++) {
			indices[0] = i;
			for (size_t j = i + 1; j < graph[i].size(); j++) {
				if (graph[i][j]) {
					indices[1] = j;
					model.addRow(2, indices, weights, 0.0, 1.0);
					addedStuff = true;
				}
			}
		}

		delete[] indices;
		delete[] weights;

		if (!addedStuff) {
			//printf("Early escape\n");
			for (size_t i = 0; i < graph.size(); i++) {
				mis.push_back(i);
			}
			return;
		}

		//printf("LP setup\n");

		// Solve
		model.primal();

		//printf("Solved!\n");

		// Extract the result
		double* sol = model.primalColumnSolution();
		for (size_t i = 0; i < graph.size(); i++) {
			if (sol[i] > 0.5) {
				mis.push_back(i);
			}
			//printf("%u: %f\n", i + 1, sol[i]);
		}
	}

	vector<vector<const Rule*>> InitPartitionRuleList(const vector<const Rule*> &rl, const unordered_map<const Rule*, int>& vr, int k, int m) {
		vector<vector<const Rule*>> results;
		for (int j = 0; j < k; j++) {
			vector<const Rule*> subset;
			for (const Rule* r : rl) {
				int minV = j * m / k;
				int maxV = (j + 1) * m / k;
				int v = vr.at(r);
				if (minV <= v && v < maxV) {
					subset.push_back(r);
				}
			}
			results.push_back(subset);
		}
		return results;
	}

	vector<vector<const Rule*>> InitPartitionTs(const vector<vector<const Rule*>> tv, int i, int beta) {
		vector<vector<const Rule*>> results;
		for (size_t j = 0; j < tv.size(); j++) {
			for (int b = 0; b < beta; b++) {
				int x = j * beta + b;
				results.push_back(tv[j]);
			}
		}
		return results;
	}

	void Projection2D::FindSimpleMIS(const vector<Rule>& rl, vector<Rule>& mis, vector<Rule>& remain) const {
		
		printf("Find Simple MIS: %lu\n", rl.size());

		vector<const Rule*> v = ToPointerVector(rl);

		int q0 = MaxClique(v).size();
		vector<vector<const Rule*>> tv(q0);

		unordered_map<const Rule*, int> vr;
		for (const Rule* r : v) {
			vr[r] = FindV(v, *r);
		}

		int i = 2;
		while (true) {
			vector<const Rule*> clique = MaxClique(v);
			int q = clique.size();
			if (q <= 1) break;
			if (--i < 0) break;
			
			SimpleIteration(v, vr, tv, q);
		}

		//printf("Remain: %u\n", v.size());
		vector<vector<const Rule*>> sv = InitPartitionRuleList(v, vr, q0, q0);
		//for (auto& s : sv) {
		//	printf("S: %u\n", s.size());
		//}
		//for (auto& t : tv) {
		//	printf("T: %u\n", t.size());
		//}

		int best = -1;
		size_t bestSize = 0;
		for (int i = 0; i < q0; i++) {
			if (sv[i].size() + tv[i].size() > bestSize) {
				best = i;
				bestSize = sv[i].size() + tv[i].size();
			}
		}
		printf("Best size: %lu\n", bestSize);
		unordered_set<const Rule*> sol(sv[best].begin(), sv[best].end());
		sol.insert(tv[best].begin(), tv[best].end());

		for (const Rule& r : rl) {
			if (sol.count(&r)) {
				mis.push_back(r);
			} else {
				remain.push_back(r);
			}
		}
	}

	void Projection2D::FindMIS(const vector<Rule>& rl, vector<Rule>& mis, vector<Rule>& remain) const {
	  //	printf("FindMIS: %lu\n", rl.size());

		int numIterations = 1; // TODO
		int beta = 20; // These numbers come from the paper, but are not explained
		//double m = 64 * log(rl.size());
		
		vector<const Rule*> v = ToPointerVector(rl);
		vector<const Rule*> clique = MaxClique(v);
		int m = clique.size();
		vector<double> lp = LPSolution(v);

		if (all_of(lp.begin(), lp.end(), [](double d) { return d <= 0.0001 || d >= 0.9999; })) {
		  //	printf("Integral solution\n");
			for (size_t i = 0; i < rl.size(); i++) {
				if (lp[i] > 0.5) {
					mis.push_back(rl[i]);
				} else {
					remain.push_back(rl[i]);
				}
			}
			return;
		}
		//	printf("Non-integral solution\n");
		exit(0);

		unordered_map<const Rule*, int> vr;
		for (const Rule* r : v) {
			vr[r] = FindV(v, *r);
		}
		vector<vector<const Rule*>> t;
		t.push_back(vector<const Rule*>());

		for (int i = 0; i < numIterations; i++) {
			Iteration(v, t, vr, i, beta, m);
		}
		printf("Left: %lu\n", v.size());
		exit(0);
		vector<vector<const Rule*>> s = InitPartitionRuleList(v, vr, m, m);
		//printf("Find best %u\n", t.size());
		vector<const Rule*> tPtrs = *max_element(t.begin(), t.end(), [](vector<const Rule*>& a, vector<const Rule*>& b) {return a.size() < b.size(); });
		//vector<const Rule*> sPtrs = *max_element(v.begin(), v.end(), [](auto a, auto b) {return a.size() < b.size(); });
		unordered_set<const Rule*> misSet;
		//if (tPtrs.size() > sPtrs.size()) {
		//	misSet.insert(tPtrs.begin(), tPtrs.end());
		//} else {
		//	misSet.insert(sPtrs.begin(), sPtrs.end());
		//}
		for (const Rule& r : rl) {
			if (misSet.count(&r)) {
				mis.push_back(r);
			} else {
				remain.push_back(r);
			}
		}
		//printf("Got set\n");
	}

	void Projection2D::SimpleIteration(vector<const Rule*>& rl, unordered_map<const Rule*, int>& vr, vector<vector<const Rule*>>& tv, size_t q) const {
		printf("Simple Iteration: q = %lu\n", q);
		vector<vector<const Rule*>> sv = InitPartitionRuleList(rl, vr, q, q);

		// Remove bad rectangles
		printf("Init size: %lu\n", rl.size());
		rl.erase(remove_if(rl.begin(), rl.end(), [=, &vr](const Rule* r) {
			return IsBadRectangle(*r, sv[vr[r]], q);
		}), rl.end());
		for (vector<const Rule*>& s : sv) {
			vector<const Rule*> s2 = s;
			s.erase(remove_if(s.begin(), s.end(), [=](const Rule* r) {
				return IsBadRectangle(*r, s2, q);
			}), s.end());
		}
		printf("Pruned size: %lu\n", rl.size());

		
		// Remove large cliques
		sv = InitPartitionRuleList(rl, vr, q, q);
		for (size_t j = 0; j < sv.size(); j++) {
			auto& s = sv[j];
			vector<const Rule*> c = MaxClique(s);
			if (c.size() >= q) {
				RemoveLargeClique(rl, c, sv, tv, j);
			}
		}

		
	}

	void Projection2D::Iteration(vector<const Rule*>& rl, vector<vector<const Rule*>>& t, const unordered_map<const Rule*, int>& vr, int i, int beta, int m) const {
		printf("Iteration %d, m = %d\n", i, m);
		int k = (int)pow(beta, i + 1);
		k = min(k, m);
		size_t cliqueThresh = 20 * m / k;

		vector<vector<const Rule*>> sv = InitPartitionRuleList(rl, vr, k, m);
		for (auto& v : sv) {
			printf("\t%lu\n", v.size());
		}
		vector<vector<const Rule*>> tv = InitPartitionTs(t, i, beta);
		int thresh = (int)(12 * 300 * m / pow(beta, i) * log(log(rl.size()))); // Numbers come from paper but are not explained
		//printf("Sv: %u, Tv: %u\n", sv.size(), tv.size());
		RemoveBadRectangles(sv, thresh);
		for (size_t j = 0; j < sv.size(); j++) {
			vector<const Rule*> clique = MaxClique(sv[j]);
			if (clique.size() > cliqueThresh) {
				RemoveLargeClique(rl, clique, sv, tv, j);
			}
		}
		//printf("Tv' %u\n", tv.size());
		t.assign(tv.begin(), tv.end());
	}

	int Projection2D::FindV(const vector<const Rule*>& v, const Rule& r) const {
		vector<const Rule*> w;
		unsigned int width = r.range[fx][HighDim] - r.range[fx][LowDim];
		for (const Rule* s : v) {
			unsigned int sWidth = s->range[fx][HighDim] - s->range[fx][LowDim];
			if (sWidth < width && NonCornerIntersection(*s, r)) {
				w.push_back(s);
			}
		}
		return MaxClique(w).size();
	}

	bool Projection2D::DoesIntersect(const Rule& r, const Rule& s) const {
		if (r.range[fx][LowDim] > s.range[fx][HighDim] || s.range[fx][LowDim] > r.range[fx][HighDim]) {
			return false;
		}
		if (r.range[fy][LowDim] > s.range[fy][HighDim] || s.range[fy][LowDim] > r.range[fy][HighDim]) {
			return false;
		}
		return true;
	}

	bool Projection2D::CornerIntersection(const Rule& r, const Rule& s) const {
		//printf("R: %u, S: %u\n", r.tag, s.tag);
		return DoesIntersect(r, s) && !NonCornerIntersection(r, s);
	}

	bool Projection2D::NonCornerIntersection(const Rule& r, const Rule& s) const {
		return DoesIntersect(r, s)
			&& ((r.range[fx][LowDim] <= s.range[fx][LowDim] && r.range[fx][HighDim] >= s.range[fx][HighDim] 
					&& (s.range[fy][LowDim] <= r.range[fy][LowDim] && s.range[fy][HighDim] >= r.range[fy][HighDim]))
				|| (s.range[fx][LowDim] <= r.range[fx][LowDim] && s.range[fx][HighDim] >= r.range[fx][HighDim]) 
					&& ((r.range[fy][LowDim] <= s.range[fy][LowDim] && r.range[fy][HighDim] >= s.range[fy][HighDim])))
			&& !(r.range[fx][LowDim] == s.range[fx][LowDim] && r.range[fx][HighDim] == s.range[fx][HighDim]
				&& r.range[fy][LowDim] == s.range[fy][LowDim] && r.range[fy][HighDim] == s.range[fy][HighDim]);
	}

	bool Projection2D::IsBadRectangle(const Rule& r, const vector<const Rule*>& rl, int thresh) const {
		int numBad = count_if(rl.begin(), rl.end(), [&](const Rule* s) { return CornerIntersection(r, *s); });
		return numBad >= thresh;
	}

	void Projection2D::RemoveBadRectangles(vector<vector<const Rule*>>& sv, int thresh) const {
		for (size_t j = 0; j < sv.size() - 2; j++) {
			vector<const Rule*> h = Union3(sv[j], sv[j + 1], sv[j + 2]);
			vector<const Rule*>& v = sv[j];
			v.erase(remove_if(v.begin(), v.end(), [&](const Rule* s) { return IsBadRectangle(*s, h, thresh); }), v.end());
		}
		// TODO: remove from rl
	}

	void Projection2D::RemoveLargeClique(
			vector<const Rule*>& rl, 
			vector<const Rule*>& clique, 
			vector<vector<const Rule*>>& sv, 
			vector<vector<const Rule*>>& tv, 
			size_t j) 
			const {
		auto top = (*max_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) { return r1->range[fy][HighDim] < r2->range[fy][HighDim]; }))->range[fy][HighDim];
		auto bottom = (*min_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) { return r1->range[fy][LowDim] < r2->range[fy][LowDim]; }))->range[fy][LowDim];
		auto right = (*max_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) { return r1->range[fx][HighDim] < r2->range[fx][HighDim]; }))->range[fx][HighDim];
		auto left = (*min_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) { return r1->range[fx][LowDim] < r2->range[fx][LowDim]; }))->range[fx][LowDim];

		const Rule* ra = nullptr;
		unsigned int minWidth = -1;
		vector<const Rule*> cTop, cBottom, cLeft, cRight;
		for (const Rule* r : clique) {
			bool isBoundary = false;
			if (r->range[fx][LowDim] == left) {
				cLeft.push_back(r);
				isBoundary = true;
			}
			if (r->range[fx][HighDim] == right) {
				cRight.push_back(r);
				isBoundary = true;
			}
			if (r->range[fy][LowDim] == bottom) {
				cBottom.push_back(r);
				isBoundary = true;
			}
			if (r->range[fy][HighDim] == top) {
				cTop.push_back(r);
				isBoundary = true;
			}
			//if (!isBoundary) ra = r;
			unsigned int width = r->range[fx][HighDim] - r->range[fx][LowDim];
			if (width < minWidth) ra = r;
		}

		const Rule* rTopLeft = *min_element(cTop.begin(), cTop.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fx][LowDim] < r2->range[fx][LowDim]; });
		const Rule* rTopRight = *max_element(cTop.begin(), cTop.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fx][HighDim] < r2->range[fx][HighDim]; });
		const Rule* rBottomLeft = *min_element(cBottom.begin(), cBottom.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fx][LowDim] < r2->range[fx][LowDim]; });
		const Rule* rBottomRight = *max_element(cBottom.begin(), cBottom.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fx][HighDim] < r2->range[fx][HighDim]; });

		rl.erase(remove(rl.begin(), rl.end(), ra), rl.end());
		tv[j].push_back(ra);
		auto x = (*max_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fx][LowDim] < r2->range[fx][LowDim]; }))->range[fx][LowDim];
		auto y = (*max_element(clique.begin(), clique.end(), [=](const Rule* r1, const Rule* r2) {return r1->range[fy][LowDim] < r2->range[fy][LowDim]; }))->range[fy][LowDim];
		vector<const Rule*> h = sv[j];//Union3(sv[j], sv[j + 1], sv[j + 2]);
		h.erase(remove_if(h.begin(), h.end(), [=](const Rule* r) {
			return !((InRange(x, r->range[fx]) && InRange(y, r->range[fy]))
				//|| CornerIntersection(*r, *ra)
				|| CornerIntersection(*r, *rTopLeft)
				|| CornerIntersection(*r, *rTopRight)
				|| CornerIntersection(*r, *rBottomLeft)
				|| CornerIntersection(*r, *rBottomRight));
		}), h.end());
		unordered_set<const Rule*> hs(h.begin(), h.end());
		rl.erase(remove_if(rl.begin(), rl.end(), [&](const Rule* r) { return hs.count(r); }), rl.end());
	}

	vector<const Rule*> Projection2D::MaxClique(const vector<const Rule*>& v) const {
		function<unsigned int(const Rule*)> fBottom = [=](const Rule* r) {return r->range[fy][LowDim]; };
		vector<unsigned int> bottoms = ConvertDistinct(v, fBottom);
		sort(bottoms.begin(), bottoms.end());

		int best = 0;
		unsigned int bestX = -1, bestY = -1;
		for (unsigned int y : bottoms) {
			unsigned int x;
			int size = CliqueHelper(v, y, x);
			if (size > best) {
				best = size;
				bestX = x;
				bestY = y;
			}
		}

		vector<const Rule*> clique(v.begin(), v.end());
		clique.erase(remove_if(clique.begin(), clique.end(), [=](const Rule* r) { return !InRange(bestX, r->range[fx]) || !InRange(bestY, r->range[fy]); }), clique.end());
		//printf("Clique size: %u\n", clique.size());
		return clique;
	}

	int Projection2D::CliqueHelper(const vector<const Rule*>& v, unsigned int y, unsigned int &x) const {
		vector<const Rule*> rl(v.begin(), v.end());
		rl.erase(remove_if(rl.begin(), rl.end(), [=](const Rule* r) { return r->range[fy][LowDim] > y || r->range[fy][HighDim] < y; }), rl.end());
		function<unsigned int(const Rule*)> fLeft = [=](const Rule* r) {return r->range[fx][LowDim]; };
		function<unsigned int(const Rule*)> fRight = [=](const Rule* r) {return r->range[fx][HighDim]; };
		vector<unsigned int> lefts = Convert(rl, fLeft);
		vector<unsigned int> rights = Convert(rl, fRight);
		sort(lefts.begin(), lefts.end());
		sort(rights.begin(), rights.end());

		int best = 0;
		x = 0;
		int current = 0;
		size_t l = 0, r = 0;
		while (l < lefts.size()) {
			if (r >= rights.size() || lefts[l] <= rights[r]) {
				current++;
				if (current > best) {
					best = current;
					x = lefts[l];
				}
				l++;
			} else {
				current--;
				r++;
			}
		}
		return best;
	}

	vector<double> Projection2D::LPSolution(const vector<const Rule*>& rl) const {
		// Find the interesting points
		//printf("LP Time!\n");
		set<unsigned int> xs, ys;
		for (const Rule* rule : rl) {
			xs.insert(rule->range[fx][LowDim]);
			//xs.insert(rule->range[fx][HighDim]);
			ys.insert(rule->range[fy][LowDim]);
			//ys.insert(rule->range[fy][HighDim]);
		}

		vector<size_t> indicesByLeft;
		vector<size_t> indicesByRight;
		for (size_t i = 0; i < rl.size(); i++) {
			indicesByLeft.push_back(i);
			indicesByRight.push_back(i);
		}
		sort(indicesByLeft.begin(), indicesByLeft.end(), [=, &rl](size_t ri, size_t rj) { return rl[ri]->range[fx][LowDim] <= rl[rj]->range[fx][LowDim]; });
		sort(indicesByRight.begin(), indicesByRight.end(), [=, &rl](size_t ri, size_t rj) { return rl[ri]->range[fx][HighDim] <= rl[rj]->range[fx][HighDim]; });

		//printf("(%u, %u)\n", xs.size(), ys.size());

		// Setup our objective
		ClpSimplex model;
		model.messageHandler()->setLogLevel(0); // No Messages
		model.resize(0, rl.size());
		for (size_t i = 0; i < rl.size(); i++) {
			model.setObjectiveCoefficient(i, -1); // Want to maximize, but CLP minimizes
			model.setColumnBounds(i, 0.0, 1.0);
		}
		
		// Add our constraints
		int* indices = new int[rl.size()];
		double* weights = new double[rl.size()];
		int loopCount = 0;

		int yi = 0;

		bool hasConstraints = false;

		for (auto y : ys) {
			//if (++yi % 100 == 0) {
			//	printf("Y: %u\n", yi);
			//}

			// TODO : something not right
			vector<size_t> indicesIn, indicesOut;
			unordered_set<size_t> current;
			for (size_t i : indicesByLeft) {
				if (InRange(y, rl[i]->range[fy])) {
					indicesIn.push_back(i);
				}
			}
			for (size_t i : indicesByRight) {
				if (InRange(y, rl[i]->range[fy])) {
					indicesOut.push_back(i);
				}
			}

			//printf("Left: %u, Right %u\n", indicesByLeft.size(), indicesByRight.size());
			//printf("In %u, Out %u\n", indicesIn.size(), indicesOut.size());

			/*for (size_t i = 0; i < rl.size(); i++) {
				if (InRange(y, rl[indicesByLeft[i]]->range[fy])) {
					indicesIn.push_back(indicesByLeft[i]);
				}
				if (InRange(y, rl[indicesByRight[i]]->range[fy])) {
					indicesOut.push_back(indicesByRight[i]);
				}
			}*/

			/*for (size_t i = 0; i < rl.size(); i++) {
				if (InRange(y, rl[i]->range[fy])) {
					indicesIn.push_back(i);
					indicesOut.push_back(i);
				}

			}
			sort(indicesIn.begin(), indicesIn.end(), [&](size_t i, size_t j) { return rl[i]->range[fx][LowDim] <= rl[j]->range[fx][LowDim]; });
			sort(indicesOut.begin(), indicesOut.end(), [&](size_t i, size_t j) { return rl[i]->range[fx][HighDim] <= rl[j]->range[fx][HighDim]; });*/

			bool hasNew = false;
			size_t in = 0, out = 0;
			while (in < indicesIn.size()) {
				if (rl[indicesIn[in]]->range[fx][LowDim] <= rl[indicesOut[out]]->range[fx][HighDim]) {
					current.insert(indicesIn[in]);
					in++;
					hasNew = true;
				} else {
					if (hasNew) {
						int n = 0;
						for (size_t i : current) {
							indices[n] = i;
							weights[n] = 1.0;
							n++;
						}
						model.addRow(n, indices, weights, 0.0, 1.0);
						hasConstraints = true;
					}
					current.erase(current.find(indicesOut[out]));
					out++;
					hasNew = false;
				}
			}
			if (hasNew) {
				int n = 0;
				for (size_t i : current) {
					indices[n] = i;
					weights[n] = 1.0;
					n++;
				}
				model.addRow(n, indices, weights, 0.0, 1.0);
				hasConstraints = true;
			}
		}
		delete[] indices;
		delete[] weights;

		//printf("LP setup\n");

		if (!hasConstraints) {
			printf("Early out\n");
			vector<double> res(rl.size(), 1.0);
			return res;
		}

		// Solve
		model.primal();

		//printf("Solved!\n");

		// Extract the result
		vector<double> result;
		double* sol = model.primalColumnSolution();
		for (size_t i = 0; i < rl.size(); i++) {
			result.push_back(sol[i]);
			//printf("%u: %f\n", i + 1, sol[i]);
		}
		

		return result;
	}
}
