#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include "SimStruct.h"
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>


void usage() {
    cerr << "./RBS"<<endl
	<< "-d <the path of the directory>"<<endl
	<< "-f <filelabel>"<<endl
	<< "-algo <algorithm> (RBS by default)"<<endl
	<< "[-e <error parameter theta> (0.0001 by default)]"<<endl
 	<< "[-a <alpha> (0.2 by default)]"<<endl
	<< "[-qn <querynum> (10 by default)]"<<endl
	<< "[-type <type of approximation error (1 or 2)> (1 by default for RBS with additive error)]"<<endl;
}

long check_inc(long i, long max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}


double calPrecision(vector<uint> algo_top, vector<pair<double,uint> > gt_answers, uint k){
    if(gt_answers.size()<k){
		return 1;
    } 
    uint hitCount = 0;
    uint real_gtsize=k;
    for(uint i=k;i<gt_answers.size();i++){
		if(gt_answers[i].first!=gt_answers[k-1].first){
			break;
		}
		else{
			real_gtsize+=1;
    	}
    }
    for(uint i=0;i<k;i++){
		for(uint j=0;j<real_gtsize;j++){
			if(algo_top[i]==gt_answers[j].second){
				hitCount+=1;
				break;
			}
		}
    }
    return (hitCount/(double)k);
}



double cal_maxError(uint vert, double *gtvalues, vector<pair<double,uint> > algoanswers){
    double maxErr=-1;
	uint *nnzarr=new uint[vert]();
	double tmp_err;
	for(uint j=0;j<algoanswers.size();j++){
		uint tmpnode=algoanswers[j].second;
		tmp_err=abs(algoanswers[j].first-gtvalues[tmpnode]);
		if(maxErr<tmp_err){
			maxErr=tmp_err;
		}
		nnzarr[tmpnode]=1;
	}
	for(uint j=0;j<vert;j++){
		if(nnzarr[j]==0){
			tmp_err=gtvalues[j];
			if(maxErr<tmp_err){
				maxErr=tmp_err;
			}
		}
	}
	delete[] nnzarr;
	return maxErr;
}


void metric(string filedir, string filelabel, Graph g, string algoname, uint querynum, double eps, double alpha, uint type){
	double avg_max_error=0,avg_pre50=0;
    uint vert=g.n;

    vector<uint> query_set;
    uint query_node;
    ifstream query_file;
    query_file.open(filedir+"query/"+filelabel+".query");
	if(!query_file){
		cout<<"ERROR: fail to read query file"<<endl;
	}

    while(query_file>>query_node){
    	query_set.push_back(query_node);
    }

    for (uint i = 0; i < querynum; ++i){
       	uint u = query_set[i];
		stringstream ss_gt,ss_gt_dir;
		ss_gt<<filedir<<"result/powermethod/"<<filelabel<<"/"<<alpha<<"/"<<u<<"_gt.txt";
		ifstream gtin;
		gtin.open(ss_gt.str());
		if(!gtin){
	    	cout<<"ERROR:unable to open groundtruth file."<<endl;
		}
	
		double *gtvalues=new double[vert]();
		vector<pair<double, uint> > gtanswers;

        uint gtCnt = 0;
        uint gt_tempNode;
        double gt_tempSim; 
		while(gtin>>gt_tempNode>>gt_tempSim){
            if(gt_tempSim>0.0){
				gtvalues[gt_tempNode]=gt_tempSim;
				gtanswers.push_back(make_pair(gt_tempSim,gt_tempNode));
				gtCnt++;
            }
		}

        sort(gtanswers.begin(), gtanswers.end(), greater<pair<double, uint> >());

		uint precision_num=50;
		if(gtCnt<50){
			precision_num=gtCnt;
		}
	
        stringstream ss_algo;
		if(type==1){
			ss_algo<<filedir<<"result/"<<algoname<<"/AddError/"<<filelabel<<"/"<<alpha<<"/"<<eps<<"/"<<u<<".txt";
        }
		else{
			ss_algo<<filedir<<"result/"<<algoname<<"/RelaError/"<<filelabel<<"/"<<alpha<<"/"<<eps<<"/"<<u<<".txt";
		}
		ifstream algoin(ss_algo.str());
        vector<uint> algoNodes;
		double *algovalues=new double[vert](); 
		bool *algocheck=new bool[vert]();
		vector<pair<double, uint> > algoanswers;
		uint realCnt = 0;
        uint algo_tempNode;
        double algo_tempSim;    
		double algo_tempOutSize;
		while(algoin>>algo_tempNode>>algo_tempSim){
            algoNodes.push_back(algo_tempNode);
        	algovalues[algo_tempNode]=algo_tempSim;
			algoanswers.push_back(make_pair(algo_tempSim, algo_tempNode));
	  		algocheck[algo_tempNode]=true;
           	realCnt++;
        }
    	sort(algoanswers.begin(),algoanswers.end(),greater<pair<double, uint> >());
		uint topknum=precision_num;
		if((uint)algoanswers.size()<topknum){
			topknum=(uint)algoanswers.size();
		}

		vector<uint> topk_algo_Nodes;
        for(uint x = 0; x < topknum; x++){
       		topk_algo_Nodes.push_back(algoanswers[x].second);
        }
		if(topknum<precision_num){
			uint tmpCnt=topknum;
			for(uint supid=0;supid<vert;supid++){
				if(tmpCnt>=precision_num){
					break;
				}
				else if(algocheck[supid]==false){
					topk_algo_Nodes.push_back(supid);
					algocheck[supid]=true;
					tmpCnt+=1;
				}
			}
		}
		avg_max_error+=cal_maxError(vert,gtvalues,algoanswers);
		avg_pre50+=calPrecision(topk_algo_Nodes,gtanswers,50);
    	delete[] algovalues;
    	delete[] gtvalues;
    	delete[] algocheck;
	}
	avg_max_error/=(double)querynum;
	avg_pre50/=(double)querynum;

    cout << "Avg MaxError="<< avg_max_error<<endl;
    cout << "Avg Precision@50="<<avg_pre50<<endl;
}


int main(int argc, char **argv){
    uint i = 1;
    char *endptr;
    string filedir="./";
    string filelabel="dblp-author";
    string algo="RBS";
    uint querynum=10;
    uint type=1;
    double alpha=0.2;
    double eps = 0.0001;
      
    while (i < argc) {
    	if (!strcmp(argv[i], "-d")) {
    	    i = check_inc(i, argc);
            filedir = argv[i];
        	if(filedir[filedir.length()-1]!='/'){
				filedir=filedir+"/";
	    	}
		} 
		else if (!strcmp(argv[i], "-f")) {
           	i = check_inc(i, argc);
           	filelabel = argv[i];
    	} 
		else if (!strcmp(argv[i], "-algo")) {
           	i = check_inc(i, argc);
        	algo = argv[i];
        } 
		else if (!strcmp(argv[i], "-e")) {
           	i = check_inc(i, argc);
           	eps = strtod(argv[i], &endptr);
        	if ((eps == 0 || eps > 1) && endptr) {
               	cerr << "Invalid eps argument" << endl;
               	exit(1);
        	}
    	} 
		else if (!strcmp(argv[i], "-qn")) {
           	i = check_inc(i, argc);
           	querynum = strtod(argv[i], &endptr);
    		if ((querynum < 0) && endptr) {
               	cerr << "Invalid querynum argument" << endl;
	           	exit(1);
           	}
    	} 
		else if (!strcmp(argv[i],"-a")){
			i = check_inc(i,argc);
			alpha=strtod(argv[i],&endptr);
			if((alpha<0) && endptr){
				cerr << "Invalid alpha argument" <<endl;
				exit(1);
			}
		}
		else if(!strcmp(argv[i], "-type")){
	    	i = check_inc(i, argc);
           	type = strtod(argv[i], &endptr);
           	if ((type!=1) && (type!=2) && endptr) {
         		cerr << "Invalid type argument" << endl;
               	exit(1);
           	}
		} 
		else {
           	usage();
           	exit(1);
        }
        i++;
    }

	cout<<"=========="<<endl;
	cout<<"Dataset: "<<filelabel<<endl;
	cout<<"alpha="<<alpha<<endl;
	cout<<"eps="<<eps<<endl;
	cout<<endl;

    SimStruct *sim=NULL;
	if(algo=="RBS"){
		sim = new RBSstruct(filedir, filelabel, eps, alpha, type);
    	if(type==1){
			cout<<endl<<endl<<"=== Approximate STPPR by RBS with additive error ==="<<endl;
		}
		else{
			cout<<endl<<endl<<"=== Approximate STPPR by RBS with relative error ==="<<endl;
		}
	}
    else if(algo=="powermethod"){
		sim = new powermethod(filedir, filelabel, eps, alpha);
    	cout<<endl<<endl<<"=== Run powermethod for the groundtruth of STPPR ==="<<endl;
	}
	cout<<endl;

    if(querynum > sim->vert){
		querynum = sim->vert;
	}

	cout<<"querynum="<<querynum<<endl;
	string queryname;	
	queryname = filedir+"query/" + filelabel + ".query";
	ifstream query;
	query.open(queryname);
	if(!query){
		cout<<"Generate the query file..."<<endl;
		stringstream ssquery_dir;
		ssquery_dir<<filedir<<"query/";
		mkpath(ssquery_dir.str());
		ofstream data_idx(queryname);
		uint* check=new uint[sim->vert]();
		vector<pair<pair<uint,uint>, double > > aliasD;
		for(uint i=0;i<sim->vert;i++){
			aliasD.push_back(make_pair(make_pair(i,i),((sim->g).getInSize(i)/(double)(sim->g).m)));
		}
		Alias alias = Alias(aliasD);
        for(uint i = 0; i < querynum; i++){
            pair<uint,uint> tempPair = alias.generateRandom(sim->R);
			uint tempnode=tempPair.first;
			while((check[tempnode]==1)||(sim->g).getInSize(tempnode)==0){
				tempPair=alias.generateRandom(sim->R);
				tempnode=tempPair.first;
			}
			check[tempnode]=1;
	     	data_idx<<tempnode<<"\n";
		}
        data_idx.close();
    	query.open(queryname);
		if(!query){
			cout<<"ERROR: cannot read the query file from "<<queryname<<endl;
			return 0;
		}
	}
	cout<<"Input the query file from: "<<queryname<<endl;

	for(uint i = 0; i < querynum; i++){
	    uint nodeId;
	    query >> nodeId;
	    cout<<i<<": "<<nodeId<<endl;
	    
	    clock_t t0=clock();
 	    sim->query(nodeId);
	    clock_t t1=clock();
	    sim->avg_time+=(t1-t0)/(double)CLOCKS_PER_SEC;
	    cout<<"Query time for node "<<nodeId<<": "<<(t1-t0)/(double)CLOCKS_PER_SEC<<" s"<<endl;
	
	    stringstream ss_dir,ss;
	    if(algo=="powermethod"){
			ss_dir<<filedir<<"result/"<<algo<<"/"<<filelabel<<"/"<<alpha<<"/";
	    }
	    else{
			if(type==1){//additive error
				ss_dir<<filedir<<"result/"<<algo<<"/AddError/"<<filelabel<<"/"<<alpha<<"/"<<eps<<"/";
	    	}
			else{//relative error
				ss_dir<<filedir<<"result/"<<algo<<"/RelaError/"<<filelabel<<"/"<<alpha<<"/"<<eps<<"/";
			}
		}
	    mkpath(ss_dir.str());
	    if(algo=="powermethod"){
	    	ss<<ss_dir.str()<<nodeId<<"_gt.txt";
	    }
	    else{
	    	ss<<ss_dir.str()<<nodeId<<".txt";
	    }
		cout<<"Write the query results in: "<<ss.str()<<endl;
	    ofstream fout;
        fout.open(ss.str());
     	fout.setf(ios::fixed,ios::floatfield);
       	fout.precision(15);
       	if(!fout){
		   	cout<<"Fail to open the writed file"<<endl;
		}
   	    for(int j = 0; j < sim->finalreserve_count; j++){
     		fout<<sim->H[0][j]<<" "<<sim->finalReserve[sim->H[0][j]]<<endl;
	   	}
       	fout.close();
   	}

	cout<<endl;
	cout<<"Avg query time: "<<sim->avg_time/(double) querynum <<" s"<<endl;

	if(algo!="powermethod"){
		cout<<endl;
		cout<<"Performance evaluation..."<<endl;
		metric(filedir, filelabel, sim->g, algo, querynum, eps, alpha, type);
	}
	cout<<endl<<endl<<endl;
	query.close();
    return 0;
}
