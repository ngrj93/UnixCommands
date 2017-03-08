//@ Nagaraj Poti - 20162010

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<errno.h>
#include<sys/types.h>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>

#define MAX_ARG_LENGTH 4097     //various Macros for errors, flags, constants
#define REVERSE_COLLATE_ERR 1
#define INV_OPT 2
#define NOT_ENOUGH_ARG 3  
#define TOO_MANY_ARG 4
#define NO_FLAG 0
#define D_FLAG 1
#define S_FLAG 2
#define C_FLAG 3
#define COMPLEMENT 1
#define NORMAL 0
#define MAX_SIZE 1000000
#define CLASS_ERROR 6
#define WRITE_ERROR 5

using namespace std;

class Translate{

private:
	char **args;  //command line arguments
	int arg_count; //number of command line arguments
	int *visited; //array to keep track of serviced commands
	bool options[4];  //set options
	char **set;  // [set 1] [set 2]
	int set_count;  //number of sets 1 or 2
	bool flag_ch;

public:
	Translate(){  //default constructor
		args=NULL;
		visited=NULL;
		set=new char*[2];
		memset(options,0,sizeof(options));
		arg_count=set_count=0;
		flag_ch=false;
	}

//---------------------------------------------------
	void handle_file_error(int errsv){  //error handling function for files
		cerr<<"Error: ";
		if(errsv==ENOENT)
			cerr<<"No such file or directory"<<endl;
		else if(errsv==EACCES)
			cerr<<"Permission denied"<<endl;
		else if(errsv==ENOTDIR)
			cerr<<"Not a directory"<<endl;
		else if(errsv==EISDIR)
			cerr<<"Is a directory"<<endl;
		else
			cerr<<errno<<endl;
		_exit(-1);
	}

//---------------------------------------------------
	void handle_other_error(int errsv){  //error handling function for all other errors
		cerr<<"Error: ";
		if(errsv==INV_OPT)
			cerr<<"Invalid option"<<endl;
		else if(errsv==NOT_ENOUGH_ARG)
			cerr<<"Missing operands --help  tr [OPTION]... SET1 [SET2]"<<endl;
		else if(errsv==TOO_MANY_ARG)
			cerr<<"Extra operand(s)"<<endl;
		else if(errsv==REVERSE_COLLATE_ERR)
			cerr<<"Range endpoints invalid"<<endl;
		else if(errsv==WRITE_ERROR)
			cerr<<"File write error"<<endl;
		else if(errsv==CLASS_ERROR)
			cerr<<"When translating, the only character classes that may appear in set2 are upper and lower"<<endl;
		_exit(-1);
	}

//---------------------------------------------------
	void open_output_dir(int index){    //recursively check for and open output directory 
		int size=strlen(args[index]);
		for(int i=0;i<size;i++){
			if(args[index][i]=='/'){
				args[index][i]='\0';
				mkdir(args[index],S_IRWXU);
				args[index][i]='/';
			}
		}
	}
//---------------------------------------------------
	void open_io_files(){             //function handles the opening of input and output files
		int input_fd,output_fd;
		for(int i=0;i<arg_count-1;i++){
			if(!strcmp(args[i],"-I") && !visited[i]){
				visited[i]=1;
				visited[i+1]=1;
				input_fd=open(args[i+1],O_RDONLY);
				if(input_fd==-1){   //error during opening input file
					int errsv=errno;
					handle_file_error(errsv);
				}
				dup2(input_fd,0);  //stdin mapped to input file if any
			}
			else if(!strcmp(args[i],"-O") && !visited[i]){
				visited[i]=1;
				visited[i+1]=1;
				open_output_dir(i+1);
				output_fd=open(args[i+1],O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);
				if(output_fd==-1){  //error during opening output file
					int errsv=errno;
					handle_file_error(errsv);
				}
				dup2(output_fd,1);  //stdout mapped to output file if any
			}
		}
	}

//---------------------------------------------------
	void check_flags(){              //function to set options given by user in cmd arguments
		for(int i=0;i<arg_count;i++){
			if(!visited[i] && args[i][0]=='-'){
				visited[i]=1;
				int size=strlen(args[i]);
				for(int j=1;j<size;j++){
					if(j==1 && args[i][j]=='-')  //enables double hyphen options to work  eg. tr --d
						continue;
					if(args[i][j]=='d')
						options[D_FLAG]=true;								 
					else if(args[i][j]=='s')
						options[S_FLAG]=true;
					else if(args[i][j]=='c'||args[i][j]=='C')
						options[C_FLAG]=true;
					else
						handle_other_error(INV_OPT);  //invalid option entered by user
				}
			}
		}
		if(options[D_FLAG]==false)
			options[NO_FLAG]=true;
	}

//---------------------------------------------------
	void check_sets(){						//function to check the validity of entered set counts
		for(int i=0;i<arg_count;i++){
			if(!visited[i]){
				visited[i]=true;
				if(set_count<2){
					set[set_count]=new char[MAX_ARG_LENGTH];
					strcpy(set[set_count],args[i]);
				}
				set_count++;
			}
		}
		if(set_count>2)
			handle_other_error(TOO_MANY_ARG);
		else if(set_count==0)
			handle_other_error(NOT_ENOUGH_ARG);
		else if(set_count==1 && ((options[D_FLAG] && options[S_FLAG])||
							(!options[D_FLAG] && options[C_FLAG] && !options[S_FLAG]))){
			handle_other_error(NOT_ENOUGH_ARG);
		}
		else if(set_count==2 && ((options[D_FLAG] && options[C_FLAG] && !options[S_FLAG])||
							(!options[S_FLAG] && options[D_FLAG]))){
			handle_other_error(TOO_MANY_ARG);
		}
	}

//---------------------------------------------------
	char* squeeze(char* buffer,int mode,char *ref){    //implementing -s option
		bool hash[256]={0};
		int ref_len=strlen(ref),buf_len=strlen(buffer);
		if(mode==NORMAL){
			for(int i=0;i<ref_len;i++)
				hash[(int)ref[i]]=true;
		}
		else{
			for(int i=0;i<256;i++)
				hash[i]=true;
			for(int i=0;i<ref_len;i++)
				hash[(int)ref[i]]=false;
		}
		char *temp=new char[MAX_SIZE];
		int cur=1;
		temp[0]=buffer[0];
		for(int i=1;i<buf_len;i++){
			if(buffer[i]!=buffer[i-1] || !hash[(int)buffer[i]]){
				temp[cur]=buffer[i];
				cur++;
			}
		}
		temp[cur]='\0';
		delete(buffer);
		return temp;
	}

//---------------------------------------------------
	char* deletion(char *buffer,int mode,char *ref){    //implementing -d option
		bool hash[256]={0};
		int ref_len=strlen(ref),buf_len=strlen(buffer);
		if(mode==NORMAL){
			for(int i=0;i<ref_len;i++)
				hash[(int)ref[i]]=true;
		}
		else{
			for(int i=0;i<256;i++)
				hash[i]=true;
			for(int i=0;i<ref_len;i++)
				hash[(int)ref[i]]=false;
		}
		char *temp=new char[MAX_SIZE];
		int cur=0;
		for(int i=0;i<buf_len;i++){
			if(!hash[(int)buffer[i]]){
				temp[cur]=buffer[i];
				cur++;
			}
		}
		temp[cur]='\0';
		delete(buffer);
		return temp;
	}

//---------------------------------------------------
	void translation(char *buffer,int mode,char *ref1,char *ref2){      //implementing translation option
		int hash[256]={0};
		int ref1_len=strlen(ref1),ref2_len=strlen(ref2),buf_len=strlen(buffer),i;
		if(mode==NORMAL){
			for(i=0;i<ref1_len && i<ref2_len;i++)
				hash[(int)ref1[i]]=ref2[i];
			while(i<ref1_len){
				hash[(int)ref1[i]]=ref2[ref2_len-1];
				i++;
			}
		}
		else{
			for(i=0;i<ref1_len;i++)
				hash[(int)ref1[i]]=-1;
			for(i=0;i<ref2_len && i<256;i++){
				if(hash[i]!=-1)
					hash[i]=ref2[i];
			}
			while(i<256){
				hash[i]=ref2[ref2_len-1];
				i++;
			}
		}
		for(i=0;i<buf_len;i++){
			if(hash[(int)buffer[i]]!=-1 && hash[(int)buffer[i]]!=0)
				buffer[i]=(char)hash[(int)buffer[i]];
		}
	}

//---------------------------------------------------
	char* expand_set(char *range,int c){        //expand the regex corresponding to set1 and set2
		char *temp=new char[MAX_ARG_LENGTH]();
		int rsize=strlen(range);
		int i=0;
		for(int u=0;u<rsize;u++){
			if(range[u]=='['){
				if(!strncmp(range+u,"[:lower:]",9)){
					strcat(temp,"abcdefghijklmnopqrstuvwxyz");
					i+=25;
					u+=8;
				}
				else if(!strncmp(range+u,"[:upper:]",9)){
					strcat(temp,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
					i+=25;
					u+=8;
				}
				else if(!strncmp(range+u,"[:digit:]",9)){
					strcat(temp,"0123456789");
					i+=9;
					u+=8;
					if(c==1)
						flag_ch=true;
				}
				else if(!strncmp(range+u,"[:space:]",9)){
					strcat(temp," \t\n");
					i+=2;
					u+=8;
					if(c==1)
						flag_ch=true;
				}
				else if(!strncmp(range+u,"[:punct:]",9)){
					strcat(temp,".,\'\'\"\"");
					i+=5;
					u+=8;
					if(c==1)
						flag_ch=true;
				}
			}
			else if(range[u]=='-'){               //checking for range 
				if(range[u-1]<range[u+1]){
					int j=0;
					do{
						temp[j+i]=temp[j+i-1]+1;
					}while(temp[i+j++]!=range[u+1]);
					i=i+j-1;
				}
				else if(range[u-1]>range[u])
					handle_other_error(REVERSE_COLLATE_ERR);
			}
			else if(range[u]=='\\'){                     //checking for tab and newline
				if(u+1<rsize && range[u+1]=='t'){
					temp[i]='\t';
					u++;
				}
				else if(u+1<rsize && range[u+1]=='n'){
					temp[i]='\n';
					u++;
				}
			}
			else
				temp[i]=range[u];
			i++;
		}
		delete range;
		return temp;
	}
							
//---------------------------------------------------
	void perform_action(){                    //processing 
		ssize_t n;
		char *buffer=new char[MAX_SIZE];
		if((flag_ch && options[NO_FLAG] && !options[S_FLAG]) || (options[NO_FLAG] && options[S_FLAG] && set_count==2))
			handle_other_error(CLASS_ERROR);
		while((n=read(0,buffer,MAX_SIZE))>0){
			if(options[NO_FLAG]){
				if(options[S_FLAG]){
					if(options[C_FLAG]){
						if(set_count==1)
							buffer=squeeze(buffer,COMPLEMENT,set[0]);
						else{
							translation(buffer,COMPLEMENT,set[0],set[1]);
							buffer=squeeze(buffer,NORMAL,set[1]);
						}
					}
					else{
						if(set_count==1)
							buffer=squeeze(buffer,NORMAL,set[0]);
						else{
							translation(buffer,NORMAL,set[0],set[1]);
							buffer=squeeze(buffer,NORMAL,set[1]);
						}
					}
				}	
				else
					translation(buffer,NORMAL,set[0],set[1]);
			}
			else{
				if(options[S_FLAG]){
					if(options[C_FLAG]){
						buffer=deletion(buffer,COMPLEMENT,set[0]);
						buffer=squeeze(buffer,NORMAL,set[1]);
					}
					else{
						buffer=deletion(buffer,NORMAL,set[0]);
						buffer=squeeze(buffer,NORMAL,set[1]);
					}
				}
				else if(options[C_FLAG])
					buffer=deletion(buffer,COMPLEMENT,set[0]);
				else
					buffer=deletion(buffer,NORMAL,set[0]);
			}
			if(write(1,buffer,strlen(buffer))<0){
				int errsv=errno;
				handle_other_error(WRITE_ERROR);
			}
			memset(buffer,0,MAX_SIZE);
		}
	}
					
//---------------------------------------------------
	void discern_semantics(){   //factory function specifying outline of operations
		open_io_files();
		check_flags();
		check_sets();  //check whether proper number of [Set 1] [Set 2] sets for input flags is entered
		for(int i=0;i<set_count;i++)
			set[i]=expand_set(set[i],i);
		perform_action();	  //perform actual translate operation
	}

//---------------------------------------------------
	void parse(int c,char **a){
		if(c<=2)
			handle_other_error(NOT_ENOUGH_ARG);  //not enough arguments for tr
		args=new char*[c-1];		//storing command line arguments in array of strings
		arg_count=c-1;
		visited=new int[arg_count];
		memset(visited,0,sizeof(visited));  //initializing visited array to 0
		for(int i=1;i<=arg_count;i++){
			args[i-1]=new char[MAX_ARG_LENGTH];
			strcpy(args[i-1],a[i]);
		}
		discern_semantics();  //process individual arguments
	}

//---------------------------------------------------
	~Translate(){
		for(int i=0;i<set_count;i++)
			delete(set[i]);
		delete(set);
		delete(visited);
	}

};

int main(int argc,char** argv){
	Translate tr;  //make new application object tr
	tr.parse(argc,argv);  //parse command line string
	return 0;
}

