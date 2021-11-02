#ifndef SIMSTRUCT_H
#define SIMSTRUCT_H

#define INT_MAX 32767

#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include "Graph.h"
#include "Random.h"
#include "alias.h"
#include <unordered_map>
#include <unordered_set>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <queue>
#include <cmath>


typedef unsigned int uint;

bool maxScoreCmp(const pair<uint, double>& a, const pair<uint, double>& b){
	return a.second > b.second;
}

class SimStruct{
	public:		
		Graph g;//Class Graph
		Random R;//Class Random
		uint vert;//# of vertice
		string filelabel;
		double alpha;
		double eps;
		double avg_time;
		uint *H[2];
		uint * U[2];
		uint *candidate_set[2];
		uint candidate_count[2];
		double *residue[2];
		double *finalReserve;
		uint finalreserve_count;

	SimStruct(string name, string file_label, double epsilon, double para_alpha){
		filelabel = file_label;
		g.inputGraph(name,file_label);
		R = Random();
		vert = g.n;
		alpha=para_alpha;
		eps = epsilon;
		avg_time = 0;
		srand(unsigned(time(0)));
		candidate_count[0]=0;
		candidate_count[1]=0;
		H[0] = new uint[vert];
		H[1] = new uint[vert];
		U[0] = new uint[vert];
		U[1] = new uint[vert];
		candidate_set[0] = new uint[vert];
		candidate_set[1] = new uint[vert];
		residue[0]=new double[vert];
		residue[1]=new double[vert];
		finalReserve = new double[vert];
		finalreserve_count=0;
		for(uint i = 0; i < vert; i++){
			residue[0][i]=0;
			residue[1][i]=0;
			finalReserve[i]=0;
			H[0][i] = 0;
			H[1][i] = 0;
			U[0][i] = 0;
			U[1][i] = 0;
			candidate_set[0][i]=0;
			candidate_set[1][i]=0;
		}
	}
	~SimStruct() {
		delete[] H[0];
		delete[] H[1];
		delete[] U[0];
		delete[] U[1];
		delete[] residue[0];
		delete[] residue[1];
		delete[] finalReserve;
		delete[] candidate_set[0];
		delete[] candidate_set[1];
	}

	virtual void query(uint u){ }
}; 

class powermethod:public SimStruct{
	public:
		powermethod(string name, string file_label, double epsilon, double para_a):
			SimStruct(name, file_label, epsilon, para_a) {
	
		}
	
		~powermethod() {
		
		}

	void query(uint u){
		for(uint j = 0; j < finalreserve_count; j++){
			finalReserve[H[0][j]] = 0;
			H[1][H[0][j]] = 0;
		}
		finalreserve_count=0;
		uint tempLevel = 0;

		residue[0][u] = 1;
		candidate_set[0][0]=u;
		candidate_count[0]=1;

		uint L=100;
		cout<<"maxIteration="<<L<<endl;

		while(tempLevel<=L){
			uint tempLevelID=tempLevel%2;
			uint newLevelID=(tempLevel+1)%2;
			uint candidateCnt=candidate_count[tempLevelID];
			if(candidateCnt==0){
				break;
			}
			candidate_count[tempLevelID]=0;
			for(uint j = 0; j < candidateCnt; j++){
				uint tempNode = candidate_set[tempLevelID][j];
				double tempR = residue[tempLevelID][tempNode];
				U[tempLevelID][tempNode]=0;
				residue[tempLevelID][tempNode] = 0;
				if(H[1][tempNode] == 0){
					H[0][finalreserve_count++] = tempNode;
					H[1][tempNode] = 1;
				}
				finalReserve[tempNode]+=alpha*tempR;
			
				if(tempLevel==L){
					continue;
				}

				uint inSize = g.getInSize(tempNode);
				for(uint k = 0; k < inSize; k++){
					uint newNode = g.getInVert(tempNode, k);
					double newOut=g.getOutSize(newNode);
					double incre = tempR* (1-alpha) / (double)newOut;

					residue[newLevelID][newNode] += incre;
					if(U[newLevelID][newNode] == 0){
						U[newLevelID][newNode] = 1;
						candidate_set[newLevelID][candidate_count[newLevelID]++]=newNode;
					}
				}
			}
			tempLevel++;
		}
	}
};


class RBSstruct:public SimStruct{
	public:
		uint type;

		RBSstruct(string name, string file_label, double epsilon, double para_alpha, int para_type):
			SimStruct(name, file_label,epsilon,para_alpha) {
			type=para_type;
		}

		~RBSstruct() {

		}

	void query(uint u){
		for(uint j = 0; j < finalreserve_count; j++){
			finalReserve[H[0][j]] = 0;
			H[1][H[0][j]] = 0;
		}
		finalreserve_count=0;
		uint tempLevel = 0;
		residue[0][u] = 1;
		candidate_set[0][0]=u;
		candidate_count[0]=1;
		uint L=(uint)ceil(log(eps)/log(1-alpha))+1;

		while(tempLevel<=L){
			uint tempLevelID=tempLevel%2;
			uint newLevelID=(tempLevel+1)%2;
			uint candidateCnt=candidate_count[tempLevelID];
			if(candidateCnt==0){
				break;
			}
			candidate_count[tempLevelID]=0;
			for(uint j = 0; j < candidateCnt; j++){
				uint tempNode = candidate_set[tempLevelID][j];
				double tempR = residue[tempLevelID][tempNode];
				U[tempLevelID][tempNode]=0;
				residue[tempLevelID][tempNode] = 0;
				if(H[1][tempNode] == 0){
					H[0][finalreserve_count++] = tempNode;
					H[1][tempNode] = 1;
				}
				finalReserve[tempNode]+=alpha*tempR;
			
				if(tempLevel==L){
					continue;
				}

				uint start_pos=g.inPL[tempNode];
				uint end_pos=g.inPL[tempNode+1];
				double ran=R.drand();
				for(uint k=start_pos;k<end_pos;k++){
					uint newNode=g.inEL[k];
					double newOut=g.in_outD[k];
					double incre = tempR* (1-alpha) / (double)newOut;

					if(type==1){//type 1: RBS with additive error
						if(sqrt(newOut)*eps<=(1-alpha)*tempR){
							residue[newLevelID][newNode] += incre;
						}
						else{
							if(sqrt(newOut)*ran*eps<=(1-alpha)*tempR){
								double ran_incre=eps/sqrt(newOut);
								residue[newLevelID][newNode] += ran_incre;
							}
							else{
								break;
							}
						}
					}
					else{//type 2: RBS with relative error
						if(newOut*eps<=(1-alpha)*tempR){
							residue[newLevelID][newNode] += incre;
						}
						else{
							if(newOut*eps*ran<=(1-alpha)*tempR){
								double ran_incre=eps;
								residue[newLevelID][newNode] += ran_incre;
							}
							else{
								break;
							}
						}
					}

					if((U[newLevelID][newNode] == 0)&&(residue[newLevelID][newNode]>eps)){
						U[newLevelID][newNode] = 1;
						candidate_set[newLevelID][candidate_count[newLevelID]++]=newNode;
					}
				}
			}
			tempLevel++;
		}
	}
};



#endif
