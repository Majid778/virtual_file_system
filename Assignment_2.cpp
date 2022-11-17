#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <list>
#include <stack>
#include <vector>
#include "Tree.hpp"
using namespace std;

VFS virtualfilesystem;
queue<Trash> bin;
inode* curr = virtualfilesystem.getRoot();

int main(){

    virtualfilesystem.loaddata("vfs.dat");
	virtualfilesystem.help();
	string user_input;
	string command;
	string parameter1;
	string parameter2;
	int MAXbin = 10;
	int binsize = 0;
	
	do
	{
        parameter1 = "";
	    parameter2 = "";
		cout<<">";
		getline(cin,user_input);
		stringstream sstr(user_input);
		getline(sstr,command,' ');
		getline(sstr,parameter1,' ');
		getline(sstr,parameter2);
		try
		{		
			if( command =="help" or command=="Help")	virtualfilesystem.help(); //Prints menu of commands
			else if( command =="pwd")	cout << pwd(curr) << endl; //Prints the path of current inode
			else if( command =="realpath" or command=="r")	virtualfilesystem.realpath(parameter1, curr); //Prints the real path
			else if( command =="ls")	virtualfilesystem.ls(curr); //Prints the children of the current inode
			else if( command =="ls_sort")	virtualfilesystem.ls_sort(*curr); //Prints the children of the current inode(in order of descending size)
			else if( command =="mkdir" or command=="m")	virtualfilesystem.mkdir(parameter1, curr);//Creates a file under the current inode location
			else if( command =="touch" or command=="t")	virtualfilesystem.touch(parameter1, stoi(parameter2),curr); //Creates a folder under the current folder
			else if(command == "cd")  virtualfilesystem.cd(parameter1, curr);//Change current directory
			else if( command =="find")	virtualfilesystem.find(parameter1, virtualfilesystem.getRoot()); //returns the path of the searched name
			else if( command =="mv")	virtualfilesystem.mv(parameter1,parameter2, curr); //moves an inode to another directory
			else if( command =="rm"){
				if(MAXbin <= binsize){//if the size is at capacity, throw error
					cout<<"Error: Bin is full"<<endl;
				}else{
				 virtualfilesystem.rm(parameter1,bin);//else remove node (add it to bin queue)
				 binsize++;
				}
			} 
			else if (command == "size") cout << virtualfilesystem.size(parameter1,curr) << " bytes" <<endl; //display total size of current directory
			else if (command == "emptybin") {//empty the bin queue
				emptybin(bin);
				binsize = 0;} //reset the current size of bin to 0 
			else if (command == "showbin") virtualfilesystem.showbin(bin); //display the details of the oldest trashed inode
			else if (command == "recover") virtualfilesystem.recover(curr,bin);// recover the most recently trashed inode back to its previous directory
			else if( command == "Exit" or command=="exit") virtualfilesystem.Exit("vfs.dat"); //saves the inodes data into a file and exits program
			else cout<<"Invalid command. Please try again"<<endl;
		}
		catch(exception &e)
		{
			cout<<"Exception: "<<e.what()<<endl;
		}

	}while(command!="exit" and command!="quit");

	return 0;
}

