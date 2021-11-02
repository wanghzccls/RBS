#  Personalized PageRank to a Target Node, Revisited. 

<br/>

## Citation
Please cite our paper when using the codes: 
```
@inproceedings{wang2020personalized,
	title={Personalized pagerank to a target node, revisited},
	author={Wang, Hanzhi and Wei, Zhewei and Gan, Junhao and Wang, Sibo and Huang, Zengfeng},
	booktitle={Proceedings of the 26th ACM SIGKDD International Conference on Knowledge Discovery \& Data Mining},
	pages={657--667},
	year={2020}
}
```

<br/>
## Tested Environment:
- Ubuntu 16.04.10
- C++ 11
- GCC 5.4.0

<br/>
## Data:
* All of the datasets used in the paper are publicly available at: 
		* https://snap.stanford.edu/data/index.html
		* https://law.di.unimi.it/datasets.php

		Additionally, we provide a toy dataset at *./dataset/*. 

* Each dataset has a unique name called filelabel (e.g. *dblp-author*, *indochina*, *orkut-links*, *it-2004* and *twitter*) . 
* Please rename the raw datasets as *${filelabel}.txt* (e.g. *dblp-author.txt*, *indochina*, *orkut-links.txt*, *it-2004.txt* and *twitter.txt*) and put them in the directory: *./dataset/*. 
* We assume that all raw datasets follow a consistent format: 
	* The number of nodes is explicitly specified in the first line of the data file. 
	* Each line following the second line indicates a directed edge in the graph. 
* We assume that all undirected graphs have been converted to directed graphs that each undirected edge appears twice in the data file. 
* We assume that the node index starts from $0$. The number of nodes is larger than the largest node index. 
* The code converts the raw data to binary file in CSR format when reading raw graphs for the first time. The converted binary data is stored in the directory: *./dataset/${filelabel}/*. 

<br/>
## Query nodes:
* When the code is invoked for the first time, it will automatically construct a query file containing 100 query nodes.
* We name the query file as ${filelabel}.query and put it in the directory: *./query/*. 

<br/>
## Execution:
We include the fundamental commands to execute the codes in the script file: *RBS-master/run_script.sh*. To automatically execute our codes, please use the following bash commands: 
```
bash run_script.sh
```

Alternatively, our codes can be executed mannually. Specifically, to compile the codes: 
```
rm STPPR
make
```
To run powermethod: 
```
./STPPR -d ./ -f dblp-author -algo powermethod -qn 10 -a 0.2
```
To run RBS with additive error: 
```
./STPPR -d ./ -f dblp-author -algo RBS -e 1e-05 -qn 10 -a 0.2 -type 1 
```
To run RBS with relative error: 
```
./STPPR -d ./ -f dblp-author -algo RBS -e 1e-05 -qn 10 -a 0.2 -type 2
```

<br/>
## Parameters:
- -d \<path of the RBS directory\> ("./" by default)
- -f \<filelabel\> ("dblp-author" by default)
- -algo \<algorithm\> ("RBS" by default)
- -e \<theta> (0.0001 by default)
- -a \<the teleport probability of PPR\> (0.2 by default)
- -qn \<querynum\> (10 by default)
- -type \<the type of RBS\> (1 by default, 1 for RBS with additive error and 2 for RBS with relative error)

<br/>
## Remark:
* *datatset/*: containing the datasets.  
* *query/*: containing the query files. 
* *result/*: containing the approximation of STPPR. 



