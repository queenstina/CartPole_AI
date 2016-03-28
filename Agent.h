#ifndef __AGENT_H__
#define __AGENT_H__

#include <random>
#include <ctime>
#include "Net.h"
#include "Board.h"

auto split = std::uniform_real_distribution<float>();
std::default_random_engine rng(time(0));

template<int n, int m>
class Agent{
private:
	Net net;
	std::vector<int> t;
	std::vector<double> v; //prevState
	//input = 4x4 = 16 states + 4 actions
	//output = Q-value
public:
	Agent(std::vector<int> t):net(t),t(t){
		
	}
	//Agent Saving/Loading (to/from file) ... To Be Added
	DIR getRand(){
		auto p = split(rng);
		if(p < 0.25){
			return R;
		}else if (p < 0.5){
			return U;
		}else if (p < 0.75){
			return L;
		}else{
			return D;
		}
		//should not reach here
		return X;	
	}	
	DIR getBest(Board<n,m>& board){
		//get best purely based on network feedforward q-value
		v= board.toVec();
		auto s = v.size();
		v.resize(s+4);//for 4 DIRs(RULD)

		//currently editing here
		float maxVal=-99999;
		DIR maxDir=R;
		for(int i=0;i<4;++i){
			v[s+i] = 1.0; //activate "action"
			auto val = net.FF(v)[0];
			if(val > maxVal){
				maxVal = val;
				maxDir = (DIR)i;
			}
			v[s+i] = 0.0;
		}
		return maxDir;
	}
	float getMax(Board<n,m>& board){
		//split this function as this serves an entirely new purpose...ish.
		v= board.toVec();
		auto s = v.size();
		v.resize(s+4);//for 4 DIRs(RULD)

		//currently editing here
		float maxVal=0;
		for(int i=0;i<4;++i){
			v[s+i] = 1.0; //activate "action"
			auto val = net.FF(v)[0];
			maxVal = val>maxVal?val:maxVal;
			v[s+i] = 0.0;
		}
		return maxVal;
	}
	DIR getNext(Board<n,m>& board, float confidence=0.5){
		return (split(rng) > confidence)? getRand() : getBest(board);
	}
	void update(DIR dir, double r, double qn){
		//r = im. reward
		//qn = q_next (SARSA or Q-Learning)
		auto alpha = 0.6;
		auto index = v.size()-4+(int)dir;
		v[index] = 1.0;

		std::vector<double> y = net.FF(v);
		y[0] = (1.0-alpha)*y[0] + alpha*(r+qn);
		net.BP(y);

		v[index] = 0.0;
	}
};

#endif