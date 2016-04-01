#ifndef __AGENT_H__
#define __AGENT_H__

#include <random>
#include <ctime>
#include "Net.h"
#include "Board.h"
#include "Utility.h"

auto split = std::uniform_real_distribution<float>();
std::default_random_engine rng(time(0));

template<int n, int m>
class Agent{
private:
	float gamma; // gamma = 1 - confidence
	Net net;
	//input = 4x4 = 16 states + 4 actions
	//output = Q-value
public:
	Agent(std::vector<int> t)
		:net(t,0.6,0.001) //learning rate = 0.6, weight decay = 0.001
	{
		gamma = 0.8; //basically, how much discount by "time"? 
	}
	//Agent Saving/Loading (to/from file) ... To Be Added
	DIR getRand(Board<n,m>& board){
		const bool* available = board.getAvailable();

		std::vector<DIR> av;
		for(int i=0;i<4;++i){
			if(available[i])
				av.push_back((DIR)i);
		}
		auto p = split(rng);

		return av[int(av.size()*p)];
	}	
	DIR getBest(Board<n,m>& board){
		//get best purely based on network feedforward q-value
		std::vector<double> v= board.toVec();
		auto s = v.size();
		v.resize(s+4);//for 4 DIRs(RULD)

		//currently editing here
		float maxVal=-99999;
		DIR maxDir=X;

		const bool* available = board.getAvailable();
		
		for(int i=0;i<4;++i){ //or among available actions
			if(available[i]){
				v[s+i] = 1.0; //activate "action"
				auto val = net.FF(v)[0];
				if(val > maxVal){
					maxVal = val;
					maxDir = (DIR)i;
				}
				v[s+i] = 0.0;

			}
		}

		return maxDir;
	}
	float getMax(Board<n,m>& board){
		//split this function as this serves an entirely new purpose...ish.
		std::vector<double> v = board.toVec();
		const bool* available = board.getAvailable();
		return getMax(v,available);
	}
	float getMax(std::vector<double>& v,const bool* available){
		auto s = v.size();
		v.resize(s+4);//for 4 DIRs(RULD)

		//currently editing here
		float maxVal=0;

		for(int i=0;i<4;++i){ //or among available actions
			if(available[i]){
				v[s+i] = 1.0; //activate "action" (r/u/l/d)
				//V = (S',A')
				auto val = net.FF(v)[0]; // Q(S',A')
				//namedPrint(val);
				maxVal = val>maxVal?val:maxVal;
				v[s+i] = 0.0; //undo activation
			}
		}
		return maxVal;
	}
	DIR getNext(Board<n,m>& board){
		//occasional random exploration
		//0.9 corresponds to "gamma" .. ish.
		return (split(rng) > 0.8)? getRand(board) : getBest(board);
	}
	void recall(){ // replay memory

	}
	void update(std::vector<double>& SA, double r, double qn,float alpha){
		//SARSA
		//State-Action, Reward, Max(next), alpha

		std::vector<double> y = net.FF(SA); //old value
		namedPrint(y[0]);
		y[0] = (alpha)*y[0] + (1.0-alpha)*(r+gamma*qn); //new value
		std::cout << "<[[" <<std::endl;
		
		//namedPrint(r);
		//namedPrint(qn);
		
		namedPrint(y[0]);

		net.BP(y);

		y = net.FF(SA);
		std::cout << " --> " <<std::endl;
		namedPrint(y[0]);
	}
};

#endif
