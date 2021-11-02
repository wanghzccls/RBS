#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
//#include <cstring>

using namespace std;

typedef unsigned int uint;


int mkpath(string s, mode_t mode=0755){
	size_t pre=0, pos;
	string dir;
	int mdret;
	if(s[s.size()-1]!='/'){
		s+='/';
	}
	while((pos=s.find_first_of('/',pre))!=string::npos){
		dir=s.substr(0,pos++);
		pre=pos;
		if(dir.size()==0){
			continue;
		}
		if((mdret=::mkdir(dir.c_str(),mode)) && errno!=EEXIST){
			return mdret;
		}
	}
	return mdret;
}


bool maxScoreCmp(const pair<double,uint>& a, const pair<double,uint>& b){
	if(a.first==b.first){
		return a.second < b.second;
	}
	else{
		return a.first < b.first;
	}
}

class Graph{
public:
	uint n;	//number of nodes
	uint m;	//number of edges

	uint* outEL;
	uint* outPL;
	uint* inEL;
	uint* inPL;
	uint* in_outD;	

	Graph(){
		
	}

	~Graph(){
		//delete[] outPL;
		//delete[] outEL;
		//delete[] inPL;
		//delete[] inEL;
		//delete[] in_outD;
	}


	void csrGraphChange(string filedir, string filelabel){
		cout<<"Change to csr format..."<<endl;
		raw_inputGraph(filedir,filelabel);
		csr_convert(filedir,filelabel);
	
		cout<<"n="<<n<<" m="<<m<<endl;
	}

	void inputGraph(string filedir, string filelabel){
		stringstream ss_attr,ss_outEL,ss_outPL,ss_inEL,ss_inPL,ss_in_outD;
		ss_attr<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.attribute";

		ifstream in_attr;
		in_attr.open(ss_attr.str());
		if(!in_attr){
			csrGraphChange(filedir,filelabel);
			cout<<"=== Graph input done! ==="<<endl;
			return;
		}
		cout<<"Read graph attributes..."<<endl;
		string tmp;
		in_attr>>tmp>>n;
		in_attr>>tmp>>m;
		cout<<"n="<<n<<" m="<<m<<endl;
		in_attr.close();

		cout<<"Read graph edges..."<<endl;
		ss_outEL<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.outEdges";
		ss_outPL<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.outPtr";
		ss_inEL<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.inEdges";
		ss_inPL<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.inPtr";
		ss_in_outD<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/"<<filelabel<<"_SortInNbrByOutdeg.in_outD";    		

		outEL=new uint[m];
		outPL=new uint[n+1];
		inEL=new uint[m];
		inPL=new uint[n+1];
		in_outD=new uint[m];

		ifstream outf(ss_outEL.str(),ios::in | ios::binary);
		outf.read((char *)&outEL[0],sizeof(outEL[0])*m);

		ifstream outpf(ss_outPL.str(),ios::in | ios::binary);
   	 	outpf.read((char *)&outPL[0],sizeof(outPL[0])*(n+1));

		ifstream inf(ss_inEL.str(),ios::in | ios::binary);
		inf.read((char *)&inEL[0],sizeof(inEL[0])*m);
 
		ifstream inpf(ss_inPL.str(),ios::in | ios::binary);
		inpf.read((char *)&inPL[0],sizeof(inPL[0])*(n+1));
 
		ifstream inoutdf(ss_in_outD.str(),ios::in | ios::binary);
		inoutdf.read((char *)&in_outD[0],sizeof(in_outD[0])*m);   

		outf.close();
		outpf.close();
		inf.close();
		inpf.close();
		inoutdf.close();

		cout<<"=== Graph input done!==="<<endl;
	}

	uint getInSize(uint vert){
		return (inPL[vert+1]-inPL[vert]);
	}
	uint getInVert(uint vert, uint pos){
		return inEL[(inPL[vert]+pos)];
	}
	uint getOutSize(uint vert){
		return (outPL[vert+1]-outPL[vert]);
	}
	uint getOutVert(uint vert, uint pos){
		return outEL[(outPL[vert]+pos)];
	}


private:	
	uint** inAdjList;
	uint** outAdjList;
	uint* indegree;
	uint* outdegree;

	//void snap_inputGraph(string filedir, string filelabel){
	void raw_inputGraph(string filedir,string filelabel){
		m=0;
		//cout<<"raw_inputGraph"<<endl;
		//string filename="./dataset/"+filelabel+".txt";
		string filename=filedir+"dataset/"+filelabel+".txt";
		cout<<"filename: "<<filename<<endl;
		ifstream infile;
		infile.open(filename);
		if(!infile){
			cout<<"ERROR: unable to open txt file: "<<filename<<endl;
			return;
		}
		
		cout<<"Read the original txt file..."<<endl;
		uint vert;
		infile>>vert;
	 	n=vert;
		uint tmpsrc,tmpdes;

		indegree=new uint[n];
		outdegree=new uint[n];
		for(uint i=0;i<n;i++){
			indegree[i]=0;
			outdegree[i]=0;
		}

		uint from;
		uint to;
		while(infile>>from>>to){
			if(from==to){
				continue;
			}
			outdegree[from]++;
			//outdegree[to]++;
			indegree[to]++;
			//indegree[from]++;
		}

		inAdjList=new uint*[n];
		outAdjList=new uint*[n];
		uint* pointer_in=new uint[n];
		uint* pointer_out=new uint[n];
		for(uint i=0;i<n;i++){
			inAdjList[i]=new uint[indegree[i]];
			outAdjList[i]=new uint[outdegree[i]];
			pointer_in[i]=0;
			pointer_out[i]=0;
		}
		infile.clear();
		infile.seekg(0);

		clock_t t1=clock();

		double tmpn;
		infile >> tmpn;
		while(infile>>from>>to){
			if(from==to){
				continue;
			}
			outAdjList[from][pointer_out[from]]=to;
			pointer_out[from]++;
			inAdjList[to][pointer_in[to]]=from;
			pointer_in[to]++;

			m+=1;
		}
		infile.close();

		clock_t t2=clock();
		cout<<"reading in graph takes "<<(t2-t1)/(1.0*CLOCKS_PER_SEC)<<" s."<<endl;
		
		cout<<"Sort the in-adjacency list of each node in the ascending order of the out-degrees..."<<endl;
		for(uint i=0;i<n;i++){
			pair<double,uint> *templist;
			templist=new pair<double,uint>[indegree[i]];
			for(uint j=0;j<indegree[i];j++){
				uint tmpNbr=inAdjList[i][j];
				templist[j]=make_pair(outdegree[tmpNbr],tmpNbr);
			}
			sort(templist,(templist+indegree[i]),maxScoreCmp);
			for(uint j=0;j<indegree[i];j++){
				inAdjList[i][j]=templist[j].second;
				//if(i<10){
				//	cout<<i<<" "<<j<<" "<<templist[j].second<<" "<<templist[j].first<<endl;
				//}
			}
		}

		delete[] pointer_in;
		delete[] pointer_out;
	}

	uint gettxtInSize(uint vert){
		return indegree[vert];
	}
	uint gettxtInVert(uint vert, uint pos){
		return inAdjList[vert][pos];
	}
	uint gettxtOutSize(uint vert){
		return outdegree[vert];
	}
	uint gettxtOutVert(uint vert, uint pos){
		return outAdjList[vert][pos];
	}

	void csr_convert(string filedir, string filelabel){
		cout<<"Write to csr file..."<<endl;
		outEL=new uint[m];
		outPL=new uint[n+1];
		inEL=new uint[m];
		inPL=new uint[n+1];
		in_outD=new uint[m];

		outPL[0]=0;
		uint outid=0;
		uint out_curnum=0;
  
		inPL[0]=0;
		uint inid=0;
		uint in_curnum=0;
 
		for(uint i=0;i<n;i++){
			outid+=gettxtOutSize(i);
			outPL[i+1]=outid;
			for(uint j=0;j<gettxtOutSize(i);j++){
				outEL[out_curnum]=gettxtOutVert(i,j);
				out_curnum+=1;
			}
			inid+=gettxtInSize(i);
			inPL[i+1]=inid;
			//cout<<i<<" "<<inPL[i+1]<<endl;
			for(uint j=0;j<gettxtInSize(i);j++){
				uint tmpnode=gettxtInVert(i,j);
				inEL[in_curnum]=tmpnode;
				in_outD[in_curnum]=gettxtOutSize(tmpnode);
				in_curnum+=1;
			}
		}

		stringstream ss_dir,ss_attr,ss_outEL,ss_outPL,ss_inEL,ss_inPL,ss_in_outD;
		ss_dir<<filedir<<"dataset/"<<filelabel<<"_SortInNbrByOutdeg/";
		mkpath(ss_dir.str());
	
		ss_attr<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.attribute";
		ofstream out_attr;
		out_attr.open(ss_attr.str());
		if(!out_attr){
			cout<<"ERROR: unable to open the attribute file: "<<ss_attr.str()<<endl;
			return;
		}
		out_attr<<"n "<<n<<"\n";
		out_attr<<"m "<<m<<"\n";
		out_attr.close();

		ss_outEL<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.outEdges";
		ss_outPL<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.outPtr";
	
		ss_inEL<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.inEdges";
		ss_inPL<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.inPtr";
		ss_in_outD<<ss_dir.str()<<filelabel<<"_SortInNbrByOutdeg.in_outD";
		

		ofstream foutEL(ss_outEL.str(),ios::out | ios::binary);
		ofstream foutPL(ss_outPL.str(),ios::out | ios::binary);
		ofstream finEL(ss_inEL.str(),ios::out | ios::binary);
		ofstream finPL(ss_inPL.str(),ios::out | ios::binary);
		ofstream fin_outD(ss_in_outD.str(),ios::out | ios::binary);
	
		foutEL.write((char *)&outEL[0],sizeof(outEL[0])*m);
		foutPL.write((char *)&outPL[0],sizeof(outPL[0])*(n+1));
		finEL.write((char *)&inEL[0],sizeof(inEL[0])*m);
		finPL.write((char *)&inPL[0],sizeof(inPL[0])*(n+1));
		fin_outD.write((char *)&in_outD[0],sizeof(in_outD[0])*m);
	
		foutEL.close();
		foutPL.close();
		finEL.close();
		finPL.close();
		fin_outD.close();
	
		return;
	}
};


