//@ Nagaraj Poti - 20162010

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<sys/types.h>
#include<errno.h>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define MAX_ARG_LENGTH 4097   //macros
#define L_FLAG 0
#define l_FLAG 1
#define c_FLAG 2 
#define m_FLAG 3
#define w_FLAG 4
#define INV_OPT 2
#define MAXLEN 1000000

using namespace std;

class WordCount{

private:
	char **args;
	int arg_count;
	int wordcount;   //count variables
	int charcount;
	int bytecount;
	int linecount;
	int longest_line;
	bool options[5];
	int* visited;

public:
	WordCount(){       //constructor initialising class members
		wordcount=charcount=bytecount=linecount=longest_line=0;
		memset(options,0,sizeof(options));
		visited=NULL;
	}

//---------------------------------------------------		
	void handle_error(int errsv){   //error handling function
		cerr<<"Error: ";
		if(errsv==INV_OPT)
			cerr<<"Invalid option"<<endl;
		_exit(-1);
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
	void check_flags(){              //function to set options given by user in cmd arguments
		for(int i=0;i<arg_count;i++){
			if(!visited[i] && args[i][0]=='-'){
				visited[i]=1;
				int size=strlen(args[i]);
				for(int j=1;j<size;j++){
					if(j==1 && args[i][j]=='-')  //enables double hyphen options to work  eg. wc --l
						continue;
					if(args[i][j]=='L')
						options[L_FLAG]=true;								 
					else if(args[i][j]=='l')
						options[l_FLAG]=true;
					else if(args[i][j]=='c')
						options[c_FLAG]=true;
					else if(args[i][j]=='m')
						options[m_FLAG]=true;
					else if(args[i][j]=='w')
						options[w_FLAG]=true;
					else
						handle_error(INV_OPT);  //invalid option entered by user
				}
			}
		}
	}
	
//---------------------------------------------------
	void perform_action(){               //counts for all input files
		for(int i=0;i<arg_count;i++){
			if(!visited[i]){
				int input_fd=open(args[i],O_RDONLY);
				if(input_fd==-1){   //error while opening input file
					int errsv=errno;
					handle_file_error(errsv);
				}
				ssize_t n;
				char buffer[MAXLEN];
				int line_length=0;
				int state=0;
				while((n=read(input_fd,buffer,MAXLEN))>0){
					for(int i=0;i<n;i++){
						charcount++;bytecount++;line_length++;
						if(buffer[i]=='\n'){
							linecount++;
							line_length--;
							longest_line=line_length>longest_line?line_length:longest_line;
							line_length=0;
						}
						if(buffer[i]==' '||buffer[i]=='\n'||buffer[i]=='\t')
							state=0;
						else if(state==0){
							state=1;
							wordcount++;
						}
					}
				}
				longest_line=line_length>longest_line?line_length:longest_line;
				close(input_fd);
			}
		}
		if(options[l_FLAG])                 //display result
			cout<<linecount<<'\t';
		if(options[w_FLAG])
			cout<<wordcount<<'\t';
		if(options[c_FLAG])
			cout<<charcount<<'\t';
		if(options[m_FLAG])
			cout<<bytecount<<'\t';
		if(options[L_FLAG])
			cout<<longest_line<<'\t';
		cout<<endl;
		if(!(options[l_FLAG] && options[w_FLAG] && options[c_FLAG] && options[m_FLAG] && options[L_FLAG]))
			cout<<linecount<<'\t'<<wordcount<<'\t'<<charcount<<'\t'<<endl;
	}
	
//---------------------------------------------------
	void discern_semantics(){   //factory function specifying outline of operations
		check_flags();
		perform_action();	  //perform actual translate operation
	}
	
//---------------------------------------------------
	void parse(int c,char **a){
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
	~WordCount(){
		delete(visited);
	}
};


int main(int argc,char** argv){
	WordCount wc;               //WordCount application object
	wc.parse(argc,argv);
	return 0;
}
