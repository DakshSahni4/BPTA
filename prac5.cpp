#include <bits/stdc++.h>
using namespace std;
void aprox_Kcenter(vector<pair<int,int>> points, int k){
    vector<int> centers;
    vector<vector<int>> distances(points.size(),vector<int> (0,points.size()));
    for(int i = 0;i<points.size();i++){
        for(int j = i+1;j<points.size();j++){
            distances[i][j] = sqrt(pow(points[i].first - points[j].first,2) + pow(points[i].second-points[j].second,2));
        }
    }
vector<int> curr_dist(points.size()); 
    for(int i = 0;i<points.size();i++){
        
    }
}
int main(){
    
}