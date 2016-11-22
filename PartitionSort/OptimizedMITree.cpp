#include "OptimizedMITree.h"

/*
std::vector<box> OptimizedMITree::RangeToBoxes(const std::vector<std::array<unsigned, 2>>& range, int key_size) const  {
	
	std::vector<box> vb; 
	
	for (int i = range.size() - 1; i >= 0; i -= key_size) {
		box b1;
		for (int j = 0; j < key_size; j++) {
			if (i - key_size + 1 + j >= 0)
				b1[j] = range[field_order[i - key_size + 1 + j]];
			else
				b1[j] = { { 0, 0 } };
		}
		vb.insert(begin(vb), b1);
	}

	//PRINT BOXes
	/*for (auto& b : vb) {
	printf("[[%u %u]] ", b);
	}
	printf("\n");
	return vb;

}*/
/*
std::vector<query> OptimizedMITree::PacketToQueries(const Packet& p, int key_size) const {
	std::vector<query> vq;
	for (int i=0;i<p.size();i++) {
		query x;
		x[0] = p[field_order[i]];
		vq.push_back(x);
	}
	for (int i = p.size() - 1; i >= 0; i -= key_size) {
		query b1;
		for (int j = 0; j < key_size; j++) {
			if (i - key_size + 1 + j >= 0)
				b1[j] = p[field_order[i - key_size + 1 + j]];
			else
				b1[j] = 0;
		}
		vq.insert(begin(vq), b1);
	}
	return vq;

}*/