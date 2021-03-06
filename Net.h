#ifndef __NET_H__
#define __NET_H__

#include "Params.h"
#include "Utility.h"
#include "Layer.h"

#include <vector>
#include <armadillo>

#include <functional>
#include <ctime>
#include <random>

using namespace arma;

//auto f = static_cast<tfun>(sigmoid);
//auto f_d = static_cast<tfun_d>(sigmoidPrime);

//auto f = static_cast<tfun>(sigmoid);
//auto f_d = static_cast<tfun_d>(sigmoidPrime);

double randNum(){
	static auto _randNum = std::bind(std::uniform_real_distribution<double>(0.0,1.0),std::default_random_engine(time(0))); //random
	return _randNum();
}
mat rms(mat& m, double eps){
	return sqrt(m+eps);
	//auto n = m.n_rows*m.n_cols;
	//return sqrt(arma::accu(m%m)/n) + eps;
}

mat lerp(mat a, mat b, double d){
	return d*a + (1-d)*b;
}

mat lerp(mat& a, mat b, double d){
	return d*a + (1-d)*b;
}

void XOR_GEN(std::vector<double>& X, std::vector<double>& Y){
	X[0] = randNum()>0.5?1:0;
	X[1] = randNum()>0.5?1:0;
	Y[0] = int(X[0]) ^ int(X[1]);
}

std::vector<std::vector<double>> mat2stdvec(mat& m){
	std::vector<std::vector<double>> V(m.n_rows);
    for (size_t i = 0; i < m.n_rows; ++i) {
        V[i] = arma::conv_to<std::vector<double>>::from(m.row(i));
    };
    return V;
}

mat stdvec2mat(std::vector<std::vector<double>>& vv){

	// flatten vv
	std::vector<double> vv_flat;
	for(auto& v : vv){
		vv_flat.insert(vv_flat.end(), v.begin(), v.end());
	}

	// from row of columns
	mat m(vv_flat);
	m.reshape(vv.size(), vv[0].size());
	return m;
}

template<int... Args>
class Net{
	private:
		mat W[sizeof...(Args) - 1]; //Weights
		mat egW[sizeof...(Args) - 1]; //gradient
		mat edW[sizeof...(Args) - 1]; //delta

		//mat g[sizeof...(Args) - 1]; //gain factor
		mat B[sizeof...(Args) - 1]; //Biases
		mat egB[sizeof...(Args) - 1]; //gradient
		mat edB[sizeof...(Args) - 1]; //delta
		Layer L[sizeof...(Args)]; //Layers
		double rho;// running average decay
		double eps;// small initial number
		double decay;// weight decay
		double loss;
		char opt;
		int n;
	public:

		Net(double rho=0.99, double eps=0.0001, double decay=0.0001, char _opt=RMSPROP, bool _init = true) // initialize weights
			:rho(rho),eps(eps),decay(decay),n(sizeof...(Args)),opt(_opt){
				arma_rng::set_seed_random();
				//TODO: get opt as arg

				if(_init){
					init(Args...);
					for(int i=0;i<n;++i){
						switch(ACTIVATION){
							case TANH:
								L[i].setT(tanh, tanhPrime);
								break;
							case SIGM:
								L[i].setT(sigmoid, sigmoidPrime);
								break;
							case RELU:
								L[i].setT(relu, reluPrime);
								break;
							case LIN:
								L[i].setT(linear, linearPrime);
								break;
							default:
								L[i].setT(tanh, tanhPrime);
								break;
						}
					}

					//L[n-1].setT(softmax, linearPrime);
					L[n-1].setT(softmax, linearPrime);
					// don't care about prime
				} 
				// rescale Weights
				/*for(int i=0;i<n;++i){
				  W[i] /= 100.0;
				  }*/
			}
		template<typename H1, typename H2>
			void init(H1 h1, H2 h2){

				int i = n-2; //sizeof...(t) = 0
				W[i].randu(h2,h1);
				egW[i].zeros(h2,h1); // set to 0
				edW[i].zeros(h2,h1); // set to 0

				//g[i].ones(h2,h1); // gain = 1

				B[i].zeros(h2);
				egB[i].zeros(h2);
				edB[i].zeros(h2);
				//		dB[i].zeros(h2);

				L[i].setSize(h1);
				L[i+1].setSize(h2);
			}
		template<typename H1, typename H2, typename... T>
			void init(H1 h1, H2 h2, T... t){
				int i = n - (2 + sizeof...(t));
				W[i].randu(h2,h1);
				egW[i].zeros(h2,h1); // set to 0
				edW[i].zeros(h2,h1); // set to 0
				//		dW[i].randu(h2,h1);

				//g[i].ones(h2,h1);

				B[i].zeros(h2);
				egB[i].zeros(h2);
				edB[i].zeros(h2);
				//		dB[i].zeros(h2);

				L[i].setSize(h1);
				init(h2,t...);
			}

		std::vector<double> FF(std::vector<double>& X){
			L[0].O() = X;
			for(size_t i=1;i<n;++i){
				L[i].transfer(W[i-1]*L[i-1].O() + B[i-1]);	
			}
			return mat2stdvec(L[n-1].O());
			//return arma::conv_to<std::vector<double>>::from(L[n-1].O());
		}	

		void BP(std::vector<std::vector<double>>& Y){
			L[n-1].G() = stdvec2mat(Y) - L[n-1].O();
			//L[n-1].G() = mat(Y) - L[n-1].O();
			//namedPrint(L[n-1].G());

			loss = 0.5 * arma::dot(L[n-1].G(), L[n-1].G());

			for(size_t i = n-2;i>=1;--i){
				L[i].G() = W[i].t() * L[i+1].G() % L[i].back_transfer();
			}

			for(size_t i=1;i<n;++i){
				mat gW = (L[i].G() * L[i-1].O().t()); //plain gradient
				mat gB = L[i].G();

				mat dW;
				mat dB;

				if(opt == ADADELTA){
					egW[i-1] = lerp(egW[i-1],gW%gW, rho);
					dW = (rms(edW[i-1],eps) / rms(egW[i-1],eps)) % gW;
					edW[i-1] = lerp(edW[i-1],dW%dW,rho);

					egB[i-1] = lerp(egB[i-1], gB%gB, rho);
					dB = (rms(edB[i-1],eps)/rms(egB[i-1],eps))%gB;
					edB[i-1] = lerp(edB[i-1],dB%dB,rho);
				}else if(opt == RMSPROP){

					egW[i-1] = lerp(egW[i-1], gW%gW, rho);
					dW = gW / (sqrt(egW[i-1] + eps));

					egB[i-1] = lerp(egB[i-1], gB%gB, rho);
					dB = gB / (sqrt(egB[i-1]) + eps);

				}else{ // SGD
					dW = 0.3 * gW;
					dB = 0.3 * gB;
					//plain
				}
				W[i-1] += dW - decay*W[i-1];//weight decay
				B[i-1] += dB - decay*B[i-1];
			}
		}

		void save(std::string f){
			for(size_t i=0;i<n-1;++i){
				W[i].save(f + "_W_" + std::to_string(i));
			}

			for(size_t i=0;i<n-1;++i){
				B[i].save(f + "_B_" + std::to_string(i));
			}
		}
		void load(std::string f){
			for(size_t i=0;i<n-1;++i){
				W[i].load(f + "_W_" + std::to_string(i));
			}
			for(size_t i=0;i<n-1;++i){
				B[i].load(f + "_B_" + std::to_string(i));
			}
		}

		Net clone(){
			Net<Args...> net(rho, eps, decay, opt, false); // do not initialize
			copyTo(net);
			return net;
		}

		void copyTo(Net& net){
			for(int i=0; i<sizeof...(Args)-1; ++i){
				// TODO : enable egW/edW etc.
				// the current purpose is to only copy over the feedforward egiths
				// so egW etc... don't matter

				//net.egW = egW;
				//net.edW = edW;

				//net.egB = egB;
				//net.edB = edB;

				net.W[i] = W[i];
				net.B[i] = B[i];
			}
			//static bool prt = true;
			//if(prt){
			//	W[0] += 1.0;
			//	W[0].print();
			//	std::cout << "---------------" << std::endl;
			//	net.W[0].print();
			//	prt = false;
			//}
		}

		double error(){
			return loss;
		}

		void print(){
			for(int i=1;i<n;++i){
				W[i-1].print();
				B[i-1].print();
			}
		}

};
#endif
