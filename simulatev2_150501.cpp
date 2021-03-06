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

using namespace std;

#define MAX_NUM  1000
#define MULTIPLE 3
#define HOPS   1
#define MAX_ENTRY 2000
#define DURATION 10

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

/*
   This part contains insert, delete, usage and total_usage methods
*/
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
            auto iter = temp2[i].find(time);
            if ( iter == temp2[i].end() )  continue;
            number += iter->second;
            count[i] -= iter->second;
            temp2[i].erase(iter);
        }
        total -= number;
    }
    void Insert( int time, vector <int> nodes, int endtime ) {
        total += nodes.size();
        for(int node : nodes) {
            count[node]++;
            auto iter = temp2[node].find(endtime);
            if( iter == temp2[node].end() ) {
                temp2[node].insert({endtime, 1});
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

/* for source random number generator*/
default_random_engine generator(time(NULL));
uniform_int_distribution<int> uniform_dis(0,DURATION);
uniform_int_distribution<int> generation(1,40);
auto uni_generation = bind(generation,generator);
auto uni_dis = bind(uniform_dis,generator);

/* for destination random number generator*/
uniform_int_distribution<int> dstGeneration(1,50);
uniform_int_distribution<int> durationGeneration(1,DURATION);
auto uniRandn = bind(dstGeneration, generator);
auto durationRandn = bind(durationGeneration, generator);

string itos(int value) {
    stringstream stream;
    stream<<value;
    return stream.str();
}

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
int Pathcost_Input(const char* filename,const int policy, const int number, vector <vector <vector <int>>> &Pathcost) {

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

	This part calculate the shortest path in the node matrix and calculate the PathLength and also Pathcost

	Input	:	policy, numberofnodes, ShortestPath, Pathcost, PathLength

	Output	:

------------------------------------------------------------------------*/
void calculate_path(int policy, int number, vector <vector <vector <int>>> &Pathcost, vector <vector <vector <int>>> &PathLength , vector <vector <vector <int>>> &ShortestPath) {
    int tmp[number][number];
    int tmp2[number][number];
    for (int i=0; i<number; i++) {
        for (int j=0; j<number; j++) {
            if(i==j) {
                ShortestPath[policy][i][j]=-1;
            }
            else    {
                ShortestPath[policy][i][j]=i;
            }
        }
    }

    for (int i=0; i<number; i++) {
        for (int j=0; j<number; j++) {
            if(i==j) {
                PathLength[policy][i][j]=0;
            }
            else    {
                PathLength[policy][i][j]=1;
            }
        }
    }

    for (int k=0; k<number; k++) {
        for (int i=0; i<number; i++) {
            for (int j=0; j<number; j++) {
                if(Pathcost[policy][i][j]>Pathcost[policy][i][k]+Pathcost[policy][k][j]) {
                    /*path length*/tmp2[i][j]=PathLength[policy][i][k]+PathLength[policy][k][j];
                    /*path cost  */Pathcost[policy][i][j]=Pathcost[policy][i][k]+Pathcost[policy][k][j];
                    /*last hop   */tmp[i][j]=ShortestPath[policy][k][j];
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
                    ShortestPath[policy][i][j]=tmp[i][j];
                }
                if (tmp2[i][j]!=-1) {
                    PathLength[policy][i][j]=tmp2[i][j];
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
            if(ch == EOF) break;
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
void GenerateMulti_Tree(traffic &MultiTraffic, map<int, traffic_node*> &traffic_tree, vector <vector <vector <int>>> ShortestPath, int policy) {
    int src;
    int x = 0;
    int y = 0;
    for (traffic_iter iter = MultiTraffic.begin(); iter != MultiTraffic.end(); iter++) {

        traffic_node* root = new traffic_node();
        root -> label = iter->first;
        traffic_tree.insert({iter->first, root});
        src = iter->first;

        for(int destination: iter->second) {
            //Use DFS to insert every nodes from root
            traffic_node* node = root;
            //1. Get the shortest path
            vector<int> path;
            ++y;
            while(destination != src ) {
                path.insert(path.begin(), destination);
                destination = ShortestPath[policy][src][destination];
            }
            x += path.size();
            //2. Using vector path, traverse the tree
            for(int cur : path) {
                auto temp = node->traffic_output.find(cur);
                if( temp == node->traffic_output.end() ) {

                    traffic_node* temp_node = new traffic_node();
                    temp_node -> label = cur;
                    node->traffic_output.insert({cur, temp_node});
                    temp = node->traffic_output.find(cur);
                }
                node = temp->second;
            }
        }

    }
    cout << "Average path length is " << (float) x/y <<endl;
    return;
}

double path_cost2(vector<int> const &path, int positions, vector <int> const &usage) {

    double costx=0.0;
    int length = path.size();
    for (int i = 0; i<length; ++i ) {
        int test = positions & 1;
        positions = positions >> 1;
        if ( ! (test) )
            continue;

        int node_usage = usage[ path[i] ];

        if(node_usage<333*2) {
            costx+=0.01;
        } else if(node_usage<666*2) {
            costx+=0.03;
        } else if(node_usage<900*2) {
            costx+=0.1;
        } else if(node_usage<1000*2) {
            costx+=0.7;
        } else {
            return 999999.99;
        }

    }
    return costx;
}

//-------------------------------------------------------------------------------
vector <int> Findpath(vector <int> const & path, int const schematic, vector <int> &usage) {
    vector <int> result;
    vector <int> temp_usage = usage;
    if( schematic == 0 ) { //Maxhop
        for( unsigned i = 0; i < path.size(); ++i) {
            if( i % HOPS == 0 || i == path.size() - 1 ) {
                if( usage [ path[i] ] >= MAX_ENTRY ) {
                    result.clear();
                    return result;
                }
                result.push_back( path.at(i) );
                ++temp_usage[ path[i] ];
            }
        }
    }
    if( schematic == 1 ) {
        double cost = 999999.99, temp_cost=999999.99;
        int positions = 0;
        int length = path.size();
        int range = myPow( 2, length );
        for( int i=myPow( 2, length-1 ); i< range; ++i ) {
            if( ! (i&1) ) {
                continue;
            }
            int temp_number = i;
            int count = 0;
            while(temp_number && count < HOPS) {
                if ( temp_number & 1 ) count = 0;
                else ++count;
                temp_number = temp_number >> 1;
            }
            if( count >= HOPS ) {
                continue;
            }
            temp_cost=path_cost2(path, i, usage);
            if( cost>temp_cost ) {
                cost=temp_cost;
                positions = i;
            }
        }
        if (cost == 999999.99) {
            return result;
        }
        for( int i=1; i <= path.size(); ++i) {
            if(positions & 1) result.push_back( path.at(i-1) );
            positions = positions >> 1;
        }
    }//Jumpflow
    usage = temp_usage;
    return result;
}

int findNexthop (int src, int dest, vector <vector <vector <int>>> ShortestPath) {
    int result = dest;
    while ( src != ShortestPath[0][src][dest] ) {
        dest = ShortestPath[0][src][dest];
        result = dest;
    };
    return result;
}
//-------------------------------------------------------------------------------
vector <int> Assign_Path(vector <int> &destination, traffic_node* root, int schematic, vector <int> &usage, vector <vector <vector <int>>> ShortestPath) {
    int src = root -> label;
    vector <int> result;
    vector <int> path;
    map < traffic_node*, vector <int> > flow_setup;
    bool loop = true;
    traffic_node* cur_src = root;
    do {

        src = cur_src->label;
        loop = true;
        flow_setup.clear();
        vector <int> temp_dest = destination;
        destination.clear();
        path.push_back( cur_src->label );

        for(int temp : temp_dest) {
            int nexthop = findNexthop (src, temp, ShortestPath);
            auto temp_node = (cur_src->traffic_output).find(nexthop);
            if ( temp == cur_src->label ) { //if the traffic stop here, skip it
                loop = false;
                //if there is only one destination, we need to push temp into
                continue;
            }

            destination.push_back ( temp );
            //Here we insert the key=node pointer, value=next node's label to map
            auto iter = flow_setup.find(temp_node->second);
            if ( iter == flow_setup.end() ) {
                vector <int> temp1 { temp };
                flow_setup.insert({ temp_node->second, temp1 });
            } else {
                iter->second.push_back( temp );
            }

        }

        if(flow_setup.size() == 0) break;
        cur_src = (flow_setup.begin())->first;



    } while( flow_setup.size() == 1 && destination.size() > 0 && loop ); //the loop is used to see whether there is any node stop in the middle
    //After quit it, we have 0 or multiple traffic nodes

    //find path by vector path, save it into result
    vector <int> temp = Findpath(path, schematic, usage);
    if( temp.empty() ) {
        result.clear();
        return result;
    } else {
        result.insert( result.begin(), temp.begin(), temp.end() );
    }
    //for (int x : result)  cout << x <<endl;

    if( flow_setup.size() >1 || !loop ) { //Recursively run it for all nodes
        for( auto iter_flow : flow_setup ) {
            vector < int > temp = Assign_Path( iter_flow.second, iter_flow.first, schematic, usage, ShortestPath );
            if ( temp.empty() ) {
                result.clear();
                return result;
            }
            result.insert( result.begin(), temp.begin(), temp.end() );
        }
    }
    // for (int x : result)  cout << x <<endl;
    return result;
}

//-------------------------------------------------------------------------------
vector<int> Handle_Flow (flows &flow, map<int, traffic_node*> &traffic_tree, int schematic, vector <int> &usage, vector <vector <vector <int>>> ShortestPath) {
//We need to add endtime to all context switches later
    map<int, traffic_node*>::iterator temp = traffic_tree.find(flow.src);
    if( temp == traffic_tree.end() ) {
        vector<int> temp;
        return temp;
    } else {
        return Assign_Path(flow.dst,temp->second, schematic, usage, ShortestPath);
    }
}
//-------------------------------------------------------------------------------
int poisson_dis(float criteria) {
    float temp = float(uni_generation());
    if (temp/40.0 < criteria) {
        return 1;
    }
    else
        return 0;
}

//-------------------------------------------------------------------------------
vector <vector <flows>> Rand_Generation_Multicast(traffic &MultiTraffic, int flownumber, int numberofnodes, float criteria, float criteria2) {
    vector <vector <flows>> result;
    int flow_num = 0;
    int time = 0;


    while (flow_num < flownumber) {
        vector <flows> temp;
        for(auto traffic_iter : MultiTraffic) {
            if(poisson_dis(criteria)) {
                flows currentFlow;
                int src = traffic_iter.first;
                destination tempDst;
                destination dstPattern = traffic_iter.second;
                //int number = 0;
                //standard distribution
                for (int iter : dstPattern) {
                 	if(float(uniRandn()/50.0) > criteria2) {
                		tempDst.push_back(iter);
                //                ++ number;
                        }
                //        if ( number >= 5) break;
                }

                //uniform distribution
                //uniform_int_distribution<int> dstGeneration1(0, dstPattern.size());
                //auto uni = bind(dstGeneration1, generator);
                //int number = uni();
                //for ( int i = 0; i < number; ++i) {
                    //if (i >= dstPattern.size() ) break;
                //    tempDst.push_back(dstPattern[i]);
                //}

                //unicast
                //tempDst.push_back( dstPattern[flow_num % dstPattern.size() ] );

                if (tempDst.size()>0) {
                    int tempEndTime = durationRandn();
                    currentFlow.src = src;
                    currentFlow.dst = tempDst;
                    currentFlow.endtime = time + tempEndTime;
                    temp.push_back(currentFlow);
                    ++flow_num;
                }

            }
            if (flow_num >= flownumber) break;
        }
        ++time;
        result.push_back(temp);
    }
    return result;
}

//-------------------------------------------------------------------------------
int main(int argc, char **argv) {
    int number=0;
    int overhead_counter = 0;
    int route_request = 0;
    int flownumber = atoi(argv[1]);	  	//
    int numberofnodes=atoi(argv[2]);    	//how many nodes in the network
    int loop_time=atoi(argv[3]);	  	//the density of flow
    float criteria_generate = 1;
    float criteria_dest = 0.9;
    int schematic=atoi(argv[4]);		//four method choose from input
    traffic MultiTraffic;
    map<int, traffic_node*> traffic_tree;
    vector<vector<int>> demand;
    int refused = 0;
    int refuseTime = 999999;
    int cur_flow = 0;
    string methodName[4];
    methodName[0]="Max_Hop_";
    methodName[1]="Jump_Flow_";
    methodName[2]="Load_Balance_";
    methodName[3]="Alpha_beta_";
    if (HOPS == 1 ) methodName[0] = "Openflow";
    string total_flow = itos(flownumber * loop_time);
    vector<vector< vector< int >>> Pathcost(1, vector< vector<int>>(numberofnodes, vector< int >(numberofnodes, 0)));
    vector<vector< vector< int >>> PathLength(1, vector< vector<int>>(numberofnodes, vector< int >(numberofnodes, 0)));
    vector<vector< vector< int >>> ShortestPath(1, vector< vector<int>>(numberofnodes, vector< int >(numberofnodes, 0)));

    string filename = (atoi(argv[5])==0) ? methodName[schematic] + total_flow + ".txt" : methodName[schematic] + total_flow + "_nodelete.txt";
    ofstream fout(filename.c_str(),ofstream::out | ofstream::app);
//----------------------------build up topology and traffic pattern-----------------------------
    if(-1 == Pathcost_Input("topology.txt", 0, numberofnodes, Pathcost) ) {
        cout<<"Error reading topology"<<endl;
        return -1;
    }

    calculate_path(0, numberofnodes, Pathcost, PathLength, ShortestPath);

    if(trafficdemand(demand)) {
        cout<<"Error reading trafficdemand"<<endl;
        return -1;
    }
    GenerateMulti(MultiTraffic,demand);
    GenerateMulti_Tree(MultiTraffic,traffic_tree, ShortestPath, 0);
    cout << "Totally src number is " << traffic_tree.size() << endl ;
    //Handleflow
    vector <vector <flows>> total_flows;
 
//-----------------------------generate flows---------------------
    for (int time = 0; time < loop_time; time++) {
        vector <vector <flows>> temp = Rand_Generation_Multicast(MultiTraffic, flownumber, numberofnodes, criteria_generate, criteria_dest);
        if ( temp.size() > total_flows.size() ) {
            total_flows.resize( temp.size() );
        }
        for ( int i = 0; i < temp.size(); ++i ) {
            total_flows[i].insert( total_flows[i].end(), temp[i].begin(), temp[i].end() );
        }
    }
    //Try to print out flows numbers
    int testnumber = 0;
    cout << "Totally " << total_flows.size() << " time slots" <<endl;
    for ( auto each : total_flows) testnumber += each.size();
    cout << "Totally generated " << testnumber << " of flows" <<endl;
    int source_number = 0;
    int destination_number = 0;
    for ( auto x1 : total_flows ) {
        for ( auto x2 : x1 ) {
            ++source_number;
            destination_number += (x2.dst).size();
        }
    }

    cout << "Source number is " << source_number << ", and destination number is " << destination_number << endl;
    int control_message = 0;
    vector <int> deleted_flow(total_flows.size(), 0);
    cout << "total number of trees are " << total_flows[0].size() << endl;
// Handle all flows and collect the result
    Switches total_nodes (numberofnodes);
    vector <int> usage = total_nodes.Usage();
    int temp_time = 0;
    int destinations = 0;
    int group_accepted = 0;
    for ( auto each_time : total_flows ) {
        int temp_dest = 0;
        for (auto each_flow : each_time) {
            ++cur_flow;
            temp_dest += (each_flow.dst).size();
            vector <int> temp_path = Handle_Flow ( each_flow, traffic_tree, schematic, usage, ShortestPath);
            if ( temp_path.empty() ) {
                ++refused;
                if ( refuseTime == 999999 )  refuseTime = temp_time;
            } else {
                total_nodes.Insert(temp_time, temp_path, temp_time + each_flow.endtime);
                usage = total_nodes.Usage();
                control_message += temp_path.size();
            }
            //fout << "flow " << cur_flow << "starts at " << temp_time << " , ends at " << temp_time + each_flow.endtime << endl;
            //for ( int x : each_flow.dst ) cout<< x << "\t";
        }
        if ( !atoi(argv[5]) ) {
            total_nodes.Destroy(temp_time);
            usage = total_nodes.Usage();
        }
        group_accepted += cur_flow - refused;
        int total_aft = total_nodes.Total_Usage();
        fout << "In time " << temp_time << ":\t" << cur_flow << " flows created\t" << cur_flow - refused << " flows accepted\t" << group_accepted << " group accepted\t" << refused << " flows refused\t" << deleted_flow[temp_time] << " flows deleted\t" << total_aft << " in switches\t" << endl;
        vector<int> statical(11, 0);
        for ( int each_node : usage ) {
            ++statical [each_node / 200];
        }
        //for ( int temp_statical : statical )  fout << temp_statical << "\t";
        //fout <<endl;
        //fout << temp_dest << "\t" << cur_flow <<endl;
        destinations += temp_dest;
        //cout << " Finished the " << temp_time << " iteration" << endl;
        ++temp_time;
        cur_flow = 0;
        refused = 0;
    }
    cout << "There are totally " << destinations << " entry"<<endl;
    cout << "refuse time is " << refuseTime << endl;
    fout << endl << "Control message is " << control_message << endl;
    fout.close();
// Output the result
    return 0;
}


/*



*/

