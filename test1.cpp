#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <cmath>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <functional>
#include <sstream>
//#include <algorithm>

using namespace std;

#define MAX_NODE 512
#define MAX_NUM  1000
#define MULTIPLE 3
#define HOPS     2
#define MAX_ENTRY 2000
#define MEAN     10
#define DURATIONTIME 10

struct traffic_node {
    int label;
    map<int, traffic_node*> traffic_output;
};

struct flows {
    int src;
    vector<int> dst;
    int endtime;
};

typedef vector <int> destination;
typedef map<int, destination> traffic;
typedef map<int, destination>::iterator traffic_iter;

string itos(int value) {
    stringstream stream;
    stream<<value;
    return stream.str();
}


class Switches {
    unsigned int total;
    vector <int> count;
public:
    Switches(unsigned int size) : total(0), count( size, 0 ), temp2( new map < int, int>[size] ), switch_size(size) {} ;
    unsigned int Total_Usage() {
        return total;
    }
    vector <int> Usage() {
        return count;
    }
    void Destroy( int time ) {
        unsigned int number = 0;
        for(int i = 0; i < switch_size; ++i) {
            for(auto iter : temp2[i]) {
                if (iter.first > time) {
                    break;
                }
                number += iter.second;
                count[i] -= iter.second;
                temp2[i].erase (iter.first);
            }
        }
        total -= number;
    }
    void Insert( int time, vector <int> nodes ) {
        total += nodes.size();
        for(int node : nodes) {
            count[node]++;
            auto iter = temp2[node].find(time);
            if( iter == temp2[node].end() ) {
                temp2[node].insert({time, 1});
            } else {
                iter->second++;
            }
        }
        return;
    }

private:
    unsigned int switch_size;
    map < int, int >* temp2; //key = time, value = entry number
};



traffic_node* init_treenode(int number) {
    traffic_node *ret = (traffic_node *)malloc(sizeof(traffic_node));
    ret->label = number;
    return ret;
}

/*------------------------------------------------------------------------
//there we design a switch data class to record all flow numbers and endtime of flows
//it contains methods: assgin(int endtime, vector<int>), output(), destroy(int time)


------------------------------------------------------------------------*/

/*------------------------------------------------------------------------

	This part calculate x to the power of p

	Input	:	x, p

	Output	:	myPow(x,p)

------------------------------------------------------------------------*/

int myPow(int x, int p) {
    if (p == 0) return 1;
    if (p == 1) return x;
    return x * myPow(x, p-1);
}
/*------------------------------------------------------------------------

	This part record the node to node cost, save it in Pathcost

	Input	:	topology.txt, numberofnodes, Pathcost[policy][][]

	Output	:	0 or -1

------------------------------------------------------------------------*/

int Pathcost_Input(const char* filename,const int policy, const int number, int[][][] &Pathcost) {

    FILE *file;
    int h=number;/*read the first line node and edge numbers*/
    /*read and skip the second line*/
    file=fopen(filename,"r");

    if(file==NULL) {
        cout<<"File doesn't exist."<<endl;
        fclose(file);
        return -1;
    }

    else {
        /*initialize traffic matrix*/

        for(int i=0; i<h; i++) {
            for(int j=0; j<h; j++) {
                if(i==j) {
                    Pathcost[policy][i][j]=0;
                }
                else {
                    Pathcost[policy][i][j]=99999;
                }
            }
        }
        /*input path cost raw data*/

        char ch=getc(file);
        int x,y,n;

        for(int i=0; ch!=EOF;)
        {
            if(ch =='\t') {
                i++;
                ch=getc(file);
            }
            else if(ch =='\n') {
                i=0;
                ch=getc(file);
            }
            else if(i==1) {
                char str[MAX_NUM]="";
                /*read the string*/
                n=0;
                while(ch!='\t') {
                    str[n]=ch;
                    ch=getc(file);
                    n++;
                }
                x=atoi(str);

            }
            else if(i==2) {
                char str[MAX_NUM]="";
                /*read the string*/
                n=0;
                while(ch!='\t') {
                    str[n]=ch;
                    ch=getc(file);
                    n++;
                }
                y=atoi(str);
            }

            else if(i==3) {
                char str[MAX_NUM]="";
                /*read the string*/
                n=0;
                while(ch!='\t') {
                    str[n]=ch;
                    ch=getc(file);
                    n++;
                }
                Pathcost[policy][x][y]=atoi(str);
            }
            else {
                ch=getc(file);
            }
        }
    }
    fclose(file);
    return 0;
}

/*------------------------------------------------------------------------

	This part calculate the shortest path in the node matrix and calculate the pathlength and also Pathcost

	Input	:	policy, numberofnodes, shortestpath, Pathcost, pathlength

	Output	:

------------------------------------------------------------------------*/
void calculate_path(int policy, int number, int[][][] &Pathcost, int[][][] &PathLength , int[][][] &ShortestPath) {
    int tmp[MAX_NODE][MAX_NODE];
    int tmp2[MAX_NODE][MAX_NODE];
    for (int i=0; i<number; i++) {
        for (int j=0; j<number; j++) {
            if(i==j) {
                shortestpath[policy][i][j]=-1;
            }
            else    {
                shortestpath[policy][i][j]=i;
            }
        }
    }

    for (int i=0; i<number; i++) {
        for (int j=0; j<number; j++) {
            if(i==j) {
                pathlength[policy][i][j]=0;
            }
            else    {
                pathlength[policy][i][j]=1;
            }
        }
    }

    for (int k=0; k<number; k++) {
        for (int i=0; i<number; i++) {
            for (int j=0; j<number; j++) {
                if(Pathcost[policy][i][j]>Pathcost[policy][i][k]+Pathcost[policy][k][j]) {
                    /*path length*/tmp2[i][j]=pathlength[policy][i][k]+pathlength[policy][k][j];
                    /*path cost  */Pathcost[policy][i][j]=Pathcost[policy][i][k]+Pathcost[policy][k][j];
                    /*last hop   */tmp[i][j]=shortestpath[policy][k][j];
                }
                else  {
                    tmp[i][j]=-1;
                    tmp2[i][j]=-1;
                }
            }
        }

        //copy matrix
        for (int i=0; i<number; i++) {
            for (int j=0; j<number; j++) {
                if (tmp[i][j]!=-1) {
                    shortestpath[policy][i][j]=tmp[i][j];
                }
                if (tmp2[i][j]!=-1) {
                    pathlength[policy][i][j]=tmp2[i][j];
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------



/*------------------------------------------------------------------------
	This part take the input trafficdemand1.txt as input and get the demand[] as E for all edges
	Input	:	trafficdemand1.txt
	Output	:	demand[9609][0:src,1:dst]
------------------------------------------------------------------------*/
int trafficdemand(vector<vector<int>> &demand) {
    FILE *file;

    /*read and skip the second line*/
    file=fopen("trafficdemand1.txt","r");
    if(file==NULL)
    {
        cout<<"File doesn't exist."<<endl;
        fclose(file);
        return -1;
    }
    else
    {
        /*initialize traffic matrix*/
        vector<int> temp;

        /*input path cost raw data*/
        char ch=getc(file);
        int n,i=0;
        for(int count=0; count<9609;)
        {
            if(ch =='\t') {
                i++;
                ch=getc(file);
            }
            else if(ch =='\n') {
                i=0;
                ch=getc(file);
            }
            else if(i==1) {
                char str[100]="";
                /*read the string*/
                n=0;
                while(ch!='\t'&&ch!='\n') {
                    str[n]=ch;
                    ch=getc(file);
                    n++;
                }
                temp.push_back(atoi(str));
            }
            else if(i==2) {
                char str[100]="";
                n=0;
                while(ch!='\t'&&ch!='\n') {
                    str[n]=ch;
                    ch=getc(file);
                    n++;
                }
                temp.push_back(atoi(str));
                demand.push_back(temp);
                temp.clear();
                count++;
            }
            else {
                ch=getc(file);
            }
        }
    }

    fclose(file);
    return 0;
}
//-------------------------------------------------------------------------------

void GenerateMulti(traffic &MultiTraffic, vector<vector<int>> &demand) {
    for(vector<int> TempTraffic: demand) {
        traffic_iter temp = MultiTraffic.find(TempTraffic[0]);
        if(temp==MultiTraffic.end()) {
            vector<int> tempVector;
            tempVector.push_back(TempTraffic[1]);
            MultiTraffic.insert({TempTraffic[0],tempVector});
        }
        else {
            temp->second.push_back(TempTraffic[1]);
        }
    }
    return;
}
//-------------------------------------------------------------------------------


void GenerateMulti_Tree(traffic &MultiTraffic, map<int, traffic_node*> &traffic_tree, int[][][] &ShortestPath, int policy) {
    for (traffic_iter iter = MultiTraffic.begin(); iter != MultiTraffic.end(); iter++) {

        traffic_node* root = init_treenode(iter->first);
        traffic_tree.insert({iter->first, root});

        for(int destination: iter->second) {
            //Use DFS to insert every nodes from root
            traffic_node* node = root;
            //1. Get the shortest path
            vector<int> path;
            int position = iter->first;
            while(position!=iter->second) {
                position = ShortestPath[policy][position][iter->second];
                path.push_back(position);
            }

            //2. Using vector path, traverse the tree
            for(int cur : path) {
                auto temp = node->traffic_output.find(cur);
                if( temp == node->traffic_output.end() ) {
                    node->traffic_output.insert({cur,init_treenode(cur)});
                    temp = node->traffic_output.find(cur);
                }
                node = temp->second;
            }
        }
    }
    return;
}
//-------------------------------------------------------------------------------
int Rand_Generation_Multicast(traffic &MultiTraffic) {

}


//-------------------------------the new objective function-------------------------------
double path_cost2(vector<int> const &path, int positions, vector <int> const &usage){

    double costx=0.0;
    int length = path.size();
    for (int i = 0; i<length; ++i ) {
        int test = positions & 1; 
        positions = positions >> 1;
        if ( ! (test) )
            continue;

        int node_usage = usage[ path[length - 1 - i] ];

        if(node_usage<333*2) {
            costx+=0.01;
        }else if(node_usage<666*2) {
            costx+=0.03;
	}else if(node_usage<900*2) {
            costx+=0.1;
        }else if(node_usage<1000*2) {
            costx+=0.7;
        }else {
            return 999999.99;
        }

    }
    return costx;
}


//-------------------------------------------------------------------------------
vector <int> Findpath(vector <int> const & path, int const schematic, vector <int> const &usage) {
    vector <int> result;
    if( schematic == 0 ) { //Maxhop
        for( unsigned i = 0; i < path.size(); ++i) {
            if( i % HOPS == 0 || i == path.size() - 1 )    result.push_back( path.at(i) );
        }
    }
    if( schematic == 1 ) {
        double cost = 999999.99, temp_cost=999999.99;
        int positions = 1;
        int length = path.size();
        int range = myPow( 2, length );
        for( int i=myPow( 2, length-1 ); i< range; ++i ) {
            if( ! (i%2) ) continue;

            int temp_number = i;
            int count = 0;
            while(temp_number && count < HOPS){
                if ( temp_number & 1 ) count = 0;
                else ++count;
                temp_number = temp_number >> 1;
            }
            if( count >= HOPS ) continue;

            temp_cost=path_cost2(path, i, usage);
            if( cost>temp_cost ) {
                cost=temp_cost;
                positions = i;
            }
        }
        for( int i=1; i <= path.size(); ++i) {
            if(positions>>i) result.push_back( path.at(i) );
        }
    }//Jumpflow
    //check the result's nodes

}
//-------------------------------------------------------------------------------
vector <int> Assign_Path(vector <int> &destination, traffic_node* root, int schematic, vector <int> &usage) {

    vector <int> result;
    vector <int> path;
    map < traffic_node*, vector <int> > flow_setup;
    bool loop = true;
    do {
        loop = true;
        flow_setup.clear();
        vector <int> temp_dest = destination;
        destination.clear();
        for(int temp : temp_dest) {
            auto temp_node = (root->traffic_output).find(temp);
            if ( temp_node == ( root->traffic_output ).end() ) { //if the traffic stop here, skip it
                loop = false;
                continue;
            }
            destination.push_back(temp);
            //Here we insert the key=node pointer, value=next node's label to map
            auto iter = flow_setup.find(temp_node->second);
            if ( iter == flow_setup.end() ) {
                vector <int> temp1 { temp };
                flow_setup.insert({ temp_node->second, temp1 });
            } else {
                iter->second.push_back( temp );
            }
        }
        path.push_back( root->label );
    } while( flow_setup.size() == 1 && destination.size() > 0 && loop ); //the loop is used to see whether there is any node stop in the middle
    //After quit it, we have 0 or multiple traffic nodes
    if( flow_setup.size() >1 ) { //Recursively run it for all nodes
        for( auto iter_flow : flow_setup ) {
            vector < int > temp = Assign_Path( iter_flow.second, iter_flow.first, schematic, usage );
            if ( temp.empty() ) {
                result.clear();
                return result;
            }
            result.insert( result.end(), temp.begin(), temp.end() );
        }
    }

    //find path by vector path, save it into result
    vector <int> temp = Findpath(path, schematic, usage);
    if( temp.empty() ) {
        result.clear();
    } else {
        result.insert( result.begin(), temp.begin(), temp.end() );
    }
    return result;
}

//-------------------------------------------------------------------------------
vector<int> Handle_Flow (flows &flow, map<int, traffic_node*> &traffic_tree, int schematic, vector <int> &usage) {
//We need to add endtime to all context switches later
    map<int, traffic_node*>::iterator temp = traffic_tree.find(flow.src);
    if( temp == traffic_tree.end() ) {
        vector<int> temp;
        return temp;
    } else {
        return Assign_Path(flow.dst,temp->second, schematic, usage);
    }
}

//-------------------------------------------------------------------------------

void test ( vector <int> const &temp){
    //temp.pop_back();
}

int main(int argc, char **argv) {
    vector <int> temp{1,2,3,4,5};
    test(temp);
    for(int x : temp)   cout << x <<endl;
    return 0;
}


