#ifndef __PCOMM_H
#define __PCOMM_H
//////////////////////////////////////
#include "stdio.h"
#include <iostream> 
#include <string>
#include <vector>
#include <sstream> 
#include <unistd.h>
#include "stdlib.h"
#include <fstream>  
using namespace std;

class Pcomm{
	private:
		FILE *fp_w;
		FILE *fp_r;
		string whole_name;
		void split(const string& s, vector<string>& sv, const char flag); 
	public:
		Pcomm(string path, string name);
		~Pcomm();
		void pwrite(vector<string> strs);
		void pread(vector<string>& out);
		void pwrite_s(char* str);
		void pwrite_s(string str);
		void pread_s(string& out);
};

void Pcomm::pwrite_s(string str){
	fp_w = fopen(whole_name.c_str(), "w");
	fprintf(fp_w, "%s", str.c_str());
	fflush(fp_w); 
	fclose(fp_w);
}

void Pcomm::pwrite_s(char* str){
	fp_w = fopen(whole_name.c_str(), "w");
	fprintf(fp_w, "%s", str);
	fflush(fp_w); 
	fclose(fp_w);
}

void Pcomm::pread_s(string& out){
	char buf[1];
	fp_r = fopen(whole_name.c_str(), "r");
	// if (fp_r == NULL) { printf("PREAD_S Fails to read the file!\n"); exit(-1);}
	if(fscanf(fp_r, "%s", buf) == 0) {printf("PREAD_S Fails to read a byte!\n"); exit(-1);};
	out = buf;
	fclose(fp_r);
}

void Pcomm::pwrite(vector<string> strs){
	string whole_str = "";
	for(int i = 0; i < strs.size(); i++)
		whole_str = whole_str + strs[i] + "\n";
	fp_w = fopen(whole_name.c_str(), "w");
	fprintf(fp_w, "%s", whole_str.c_str());
	fflush(fp_w); 
	fclose(fp_w);
}

void Pcomm::pread(vector<string>& out){
	out.clear();
	ifstream in;
	in.open(whole_name.c_str(), ifstream::in);      
    if(!in) { printf("PREAD Fails to read the file!\n"); exit(-1); }
    
    std::string line;
    while(getline(in, line)) {
        std::stringstream ss(line);
        std::string key;
        ss >> key;
        out.push_back(key);
    }
    in.close();
}

void Pcomm::split(const string& s, vector<string>& sv, const char flag) {
    sv.clear();
    istringstream iss(s);
    string temp;
    while (getline(iss, temp, flag)) {
		
        sv.push_back(temp);
    }
    return;
}

Pcomm::Pcomm(string path, string name){
	whole_name = path + name;
	fp_w = fopen(whole_name.c_str(), "w");
	fclose(fp_w);
	fp_r = fopen(whole_name.c_str(), "r");
	if (fp_r == NULL) { printf("Fails to read the file!\n"); exit(-1);}	
	fclose(fp_r);
}

Pcomm::~Pcomm(){
	
}


#endif
