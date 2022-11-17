#include<list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include<exception>
#include <stack>
#include <vector>
#include <queue>

using namespace std;


class Error: public exception//Error class
{
	public:
		virtual const char* what() const throw()
		{
			return "Error!"; 
		}
};

string Currentdate(){ //Function to find the current date so that it can be used when creating new inodes
	string date = ""; 
	time_t date_ = time(0); 
	tm* current = localtime(&date_);
	date += to_string((current->tm_mday))  + "-" + to_string((current->tm_mon+1)) + "-" + to_string((current->tm_year-100));
	return date;
}

class inode{//All the data will be stored in the form of a tree made up of inodes
	private:
		friend class VFS;
	public:
		bool isFile;
		string date;
		string name;
		int size;
		inode* parent;
		vector<inode> children;//vector of inodes that are children of the parent inode
		inode();
		inode(bool isFile,string name, int size, string date, inode* parent);//constructor of inode class
		~inode();//destructor of inode class
		vector<inode> getchildren(){//getter function for children
			return children;
		}
		void addchild(inode child){//used to add a child to inode
			children.push_back(child);
		}
		void removechild(inode* child){//used to remove child from children vector
			int index = -1;
			for(auto i:children){//iterate to get the index of inode that is to be removed
				index++;
				if(i.name == child->name){
					break;
				}
			}
			children.erase(children.begin() + index);//remove inode at the index
		}
		void setparent(inode *new_parent);//sett function for parent
};

inode::inode(){//default constructor for inode
	this->isFile = 0;
    this->date = Currentdate();
	this->name = "";
	this->size = 0;
    this->parent = NULL;	
}
inode::inode(bool isFile,string name, int size, string date, inode* parent){//non-default constructor
	this->isFile = isFile;
    this->date = date;
	this->name = name;
	this->size = size;
    this->parent = parent;
}
inode::~inode(){//The destructor keeps giving me an error when I try to run it. I could not figure out why. Every thing else from my code works as intended.
	// if (!isFile){//delete children;
	// 	for(int i = 0; i<children.size()-1;i++){
	// 	delete &children[i];}
	// }
}
string pwd(inode* curr){//returns a string which is the full file path
	stack<string> mystack;
	string path = "";
	if(curr->parent == NULL){//if curr is the root
		return "/";
	}
	while(curr->parent != NULL){//iterates through the parents of the current node until it reaches the root
		mystack.push(curr->name);
		curr = curr->parent;
	}
	while(!mystack.empty()){
		path += "/" + mystack.top();//addition of '/' to the string so that it is a path
		mystack.pop();
	}
	return path;
}

void inode::setparent(inode *new_parent){//Fucnction to set a new parent to an inode
	this->parent = new_parent;
}
class Trash{//A class for storing the inode data after it has been removed by the rm function
	public:
		inode node;
		string prev_path;
		vector<inode*> prev_positionlist; //Used to store previous inode positions of the inode itself and all its children
		int prev_size;
		Trash(inode node,string prev_path,int prev_size){//constructor for trash class
			this->node = node;
			this->prev_path = prev_path;
			this->prev_size = prev_size;
		}
};

class VFS{//The main tree class that links all the inodes 
	private: 
		inode* root;//The root inode
		vector<inode*> positionlist;//A vector of pointers to all the currently existing inodes
		friend class inode;
	public:
		VFS();
		~VFS();
		inode* previous;
		inode* getRoot();
		void help();
		void realpath(string name, inode* curr);
		void ls(inode *curr); 
		void ls_sort(inode &curr); 
		void mkdir(string foldername, inode* curr);
		void Mkdir(string file_name,string date);
		void touch(string file_name,int size , inode* curr);
		void Touch(string file_name,int size,string date);
		void cd(string foldername, inode* &curr);
		void find(string name, inode* curr);
		void mv(string filename, string foldername, inode* curr);
	 	void rm(string name,queue<Trash> &bin);
		int size(string name,inode* curr); 
		int Size(inode* curr); 
		void showbin(queue<Trash> bin);
		void recover(inode* curr,queue<Trash> bin); 
		void Exit(string filename);
		void dumpData(inode* current, ofstream &newFile); 
		void loaddata(string filename);
		void dumpdata(ofstream &outFile, inode* curr);
		void print_all(inode* root);
		inode* getchild(string name, inode* curr);
		inode* getposition(string name);
		void removeposition(inode* node);
};

void VFS::print_all(inode* root){ //Function to iterate through the position list and print the path of each inode
	for(auto i: positionlist){
		cout<<pwd(i)<<endl;
	}
}
inode* VFS::getchild(string name, inode* curr){//searches the position list for a child with a specific name and returns it
	for(auto i: positionlist){
		if(name == i->name){
			return i;
		}
	}
	cout<<"Error: File not found"<<endl;//if the file is not found print error message
	throw Error();
}
VFS::VFS(){//Constructor of VFS class
	root = new inode(false,"/",0,Currentdate(),NULL);
	previous = getRoot();
}
VFS::~VFS(){//Destructor of VFS class
	delete root;
}
inode* VFS::getRoot(){//Getter function for root
	return root;
}
void VFS::mkdir(string foldername,inode* curr){//Function for creating an folder inode under current inode provided in arguments
	vector<inode> curr_folder = curr->getchildren();
	if(foldername == ""){//if nothing was entered for the parameter, throw error
		cout<<"Error: Name provided is not a valid foldername"<<endl;
		throw Error();
	}
	for(char i: foldername){ //checking if ther name is alphanumeric,if not throw error
		if (!isalnum(i)){ 
			cout<<"Error: Name provided is not a valid foldername"<<endl;
			throw Error();
		}
	}
	for(char i: foldername){//checking if given name has a space,if not throw error
		if(i == ' '){
			cout<<"Error: Name provided is not valid foldername"<<endl;//if it has a space then throw error message
			throw Error();
		}
	}
	for(char i: foldername){//checking if given name is a file name
		if(i == '.'){
			cout<<"Error: Name provided is not a valid foldername"<<endl;//if it is a filename throw error message
			throw Error();
		}
	}
	for(auto i: curr_folder){
		if(i.name == foldername){//Checking if the foldername is unique
			cout<<"Error: Folder has an element with the same name"<<endl;
			throw Error();
		}
	}
	if(curr->isFile){//checking if the current inode is of type folder
		cout<<"Error: Current inode is not a folder"<<endl;
		throw Error();
	}else{
		inode* folder = new inode(false, foldername, 10, Currentdate(),curr);//creating new inode. inode is linked to parent from by using the inode contructor
		curr->addchild(*folder);//Adding the new folder inode to the children list of its parent(curr inode)
		positionlist.push_back(folder);//adding the new inode to the positionlist
		cout<<"Folder with the name "<< foldername << " has been created"<<endl;	
	}
}
void VFS::touch(string file_name,int size, inode* curr){//Function for creating an file inode under current inode provided in arguments
	vector<inode> curr_folder = curr->children;
	if(file_name == ""){
		cout<<"Error: Name provided is not a valid file_name"<<endl;
		throw Error();
	}
	for(char i: file_name){ //checking if the name is alphanumeric,if not throw error
		if (!isalnum(i) && i != '.'){ 
			cout<<"Error: Name provided is not a valid file_name"<<endl;
			throw Error();
		}
	}
	for(char i: file_name){//checking if given name has a space,if not throw error
		if(i == ' ' || file_name == ""){
			cout<<"Error: Name provided is not valid file_name"<<endl;//if it has a space then throw error message
			throw Error();
		}
	}
	bool isFile = 0;//Variable used to check if the given filename is vaild (needs to have '.')
	for(char i: file_name){//checking if the given filename is vaild by iterating through each char of filename and check for '.'
		if(i == '.'){
			isFile = true;
			break;
		}
	}if(isFile == false){//if no '.' is found then throw error
		cout<<"Error: Name provided is not a filename"<<endl;
		throw Error();
	}
	for(auto i: curr_folder){//Checking if file name is unique
		if(i.name == file_name){
			cout<<"Error: Folder has an element with the same name"<<endl;
			throw Error();
		}
	}if(curr->isFile){//Checking id the current inode is of type folder,if not throw error
		cout<<"Error: Parent inode is not a folder"<<endl;
		throw Error();
	}else{
		inode* file = new inode(true, file_name, size, Currentdate(),curr);//creating new inode. inode is linked to parent from by using the inode contructor
		curr->addchild(*file);//Adding the new file inode to the children list of its parent(curr inode)
		positionlist.push_back(file);//adding the new inode to the positionlist
		cout<<"File with the name "<< file_name << " has been created"<<endl;
	}
}
void VFS::realpath(string name, inode* curr){//Prints the path of a node when given its name and under the current directory
	string realpath = pwd(curr) + "/" + name;
	cout<<realpath<<endl;
}
void VFS::ls(inode *curr){//lists the children inodes of the current directory
	for(auto i:curr->getchildren()){//iterates through current inodes children
		if(i.isFile){//different types depending on the type of the inode
			cout<<"file"<<"  ";
		}else{
			cout<<"dir"<<"  ";
		}
		cout<<i.name<<"  "<<i.size<<"bytes  "<<i.date<<endl;//printing the details of the child inode
	}
}
void VFS::ls_sort(inode &curr){//Same as ls but prints the children in order based on size
    for (int i = 0; i < curr.getchildren().size()-1; i++){//Bubble sort algorithim
    	for (int j = 0; j < curr.getchildren().size()-i-1; j++){
        	if (curr.getchildren()[j].size < curr.getchildren()[j+1].size){//If next element less than current, then swap them
            inode temp = curr.children[j];
			curr.children[j] = curr.children[j+1];
			curr.children[j+1] = temp;
			}
		}
	}
	ls(&curr);//After sorting call ls to display children in descending order
}	
void VFS::removeposition(inode* node){//Function used to remove inodes from position list
	int index = -1;
	for(auto i: positionlist){//for loop used to get the index of the inode in the poistiolist vector
		index++;
		if(i->name == node->name){
			break;
		}
	}
	positionlist.erase(positionlist.begin() + index);//using the index to remove the inode
}
void VFS::cd(string foldername, inode* &curr){//Function used to navigate the tree i.e. change directories
	inode* temp = nullptr;
	bool namepass = 0;//Variable to check if the entered parameter is the '..' command or not
	if(foldername == ".."){ //If entered parameter is a command to go to parent of current inode
		namepass = 1;//set check to positive
		if (curr->parent == NULL){ //Checking if the current inode is the root
			cout << "Error: Already at root" << endl;//return error if the current element is the root
			cout<<"current directory: "<<pwd(curr)<<endl;//print current path
			return ;//leave the function
		}else{
			previous = curr;//setting the previous inode to current inode for later use in the '-' command
			curr = curr->parent;//setting the current node to its parent node
			cout<<"Current directory: "<<pwd(curr)<<endl;//print current path
			return ;//leave the function
		}
	}
	if(namepass == 0){//if entered parameter is not the '..' command
		for(char i: foldername){//check if the parameter is a proper foldername
			if(i == '.'){
				cout<<"Error: Name provided is not a foldername"<<endl;
				throw Error();
			}
		}
	}
	if (foldername == " "|| foldername == "" || foldername == "cd"){//command to go to the root
		previous = curr;//setting the previous inode to current inode for later use in the '-' command
		curr = getRoot();//setting the current inode to root
		cout<<"current directory: "<<pwd(curr)<<endl;//print current path
		return ;//leave the function
	}else if(foldername == "-"){//command to go to the previous directory
		temp = curr;
		curr = previous;
		previous = temp;//swapping current and previous
		cout<<"current directory: "<<pwd(curr)<<endl;//print current path
		return ;//leave the function
	}
	else if(foldername[0] == '/'){//checking if the parameter is a path
		vector<string> name;//Vector to store the file directory names
		string temp;
		stringstream strStream(foldername);
		while(getline(strStream, temp,'/')){//extracting the characters before the filename
			name.push_back(temp);
		}
		getline(strStream,temp);//left with the filename only
		name.push_back(temp);//The last element of the name vector is the file name
		previous = curr;//setting the previous inode to current inode for later use in the '-' command
		curr = getchild(name.back(),getRoot());//get the inode of the path and set it to the current inode
		cout<<"current directory: "<<pwd(curr)<<endl;//print current path
		return ;//leave the function
	}
	else{//the normal case i.e. entered parameter is a proper foldername
		previous = curr;//setting the previous inode to current inode for later use in the '-' command
		curr = getchild(foldername,getRoot());//search for the folder and set it current inode
		cout<<"current directory: "<<pwd(curr)<<endl;//print current path
		return ;//leave the function
	}
	return ;//leave the function
}
void VFS::mv(string filename, string foldername, inode* curr){//function used to move a file/folder to another directory
	bool ischildpresent = 0;//variable to check if a certain child is present
	if(filename[0] == '/'){// if the parameter is a path, extract the filename from it
		vector<string> name;
		string temp;
		stringstream strStream(filename);
		while(getline(strStream, temp,'/')){
			name.push_back(temp);
		}
		getline(strStream,temp);
		name.push_back(temp);
		filename = name.back();
	}
	for(char i: foldername){//check if the parameter is a proper foldername
		if(i == '.'){
			cout<<"Error: Name provided is not a foldername"<<endl;
			throw Error();
		}
	}bool isFile = 0;//used to flag it is a filename or not
	for(char i: filename){//check if the parameter is a proper filename
		if(i == '.'){
			isFile = true;
			break;
		}
	}if(isFile == false){//if it is not a filename, throw error
		cout<<"Error: Name provided is not a filename"<<endl;
		throw Error();
	}for(auto i:curr->getchildren()){//check if the child exists under the current inode
		if(i.name == filename){
			ischildpresent = 1;
			break;
		}
	}if(ischildpresent == 0){//if it doesnt exist then throw error
		cout<<"Error: File/Folder not found in current directory"<<endl;
		throw Error();
	}
	inode* file = getchild(filename,curr);//find the file/child inode then set it to the file inode 
	file->parent->removechild(file);//remove the link between the previous parent of the inode
	inode* folder = getchild(foldername,curr);//find the folder/parent inode then set it to the file inode 
	file->setparent(folder);//set folder as the new parent of the file
	folder->addchild(*file);//add the file as a child of the folder inode
	cout<<"File/Folder has been moved to: "<<pwd(file)<<endl;//confirmation message
}
int VFS::size(string name, inode* curr){//used to return the size of a inode by searching for its name
	if(name == "/"){//if the name is the root
		return Size(getRoot());//return the size of the root(the whole tree)
	}
	if(name == "" || name == " "){//if there is no parameter then return the size of the current directory
		return Size(curr);
	}
	return Size(getposition(name));//search for the current inode from the position list and return its size
}
int VFS::Size(inode* curr){//Function used to actually calculate the total size of file/folder
	int totalsize = 0;
	string temp,name = curr->name;
	if(curr->parent == NULL){//if the current inode is the root
		for(auto i: positionlist){
			totalsize += i->size;//sum up all the inodes in the position list
		}
		return totalsize;
	}
	if(!curr->isFile){//if the current inode is a folder
		for (auto i: positionlist){//searches the position list for any inodes that have the current inodes name in its path
			stringstream strStream(pwd(i));//which means that the nodes found are either children of the current inode or the inode itself
			while(getline(strStream, temp,'/')){//extracting the filename from path (if parameter is a path)
				if(temp == name){
					totalsize += i->size;//return total size of folder and its children
				}
			}
		}
	}else if(curr->isFile){//if the current inode is a file
		return curr->size;
	}
	return totalsize;
}
void VFS::rm(string name,queue<Trash> &bin){//Function used to remove an inode (and add it to the bin queue)
	
	if(name[0] == '/'){//if the given name is a path, then extract the filename from it
		vector<string> names;
		string temp;
		stringstream strStream(name);
		while(getline(strStream, temp,'/')){
			names.push_back(temp);
		}
		getline(strStream,temp);
		names.push_back(temp);
		name = names.back();
	}
	inode* curr = getchild(name,getRoot());//find the inode with the same name and set it to current inode
	int prev_size = Size(curr);//saving the size of the inode for later use in showbin function
	curr->parent->removechild(curr);//removing inode from parents children vector
	Trash trash(*curr,pwd(curr),prev_size);//create a trash object
	trash.prev_positionlist.push_back(curr);
	removeposition(curr);//remove from the position list
	string temp;
	for (auto i: positionlist){//iterate through position list and remove all the current inodes children 
		stringstream strStream(pwd(i));
		while(getline(strStream, temp,'/')){
			if(temp == name){
				trash.prev_positionlist.push_back(i);
				removeposition(i);
			}
		}
	}
	bin.push(trash);//add the trash object to the bin queue
	cout<< name <<" has been removed"<<endl;//confirmation message
}
void VFS::showbin(queue<Trash> bin){//Function that displays oldest node in bin
	if(!bin.empty()){//if bin is not empty display the oldest inode details
		inode trash = bin.front().node;
		cout<<"Name: "<< trash.name<<endl;
		cout<<"Date created: "<< trash.date<<endl;
		cout<<"Size: "<< bin.front().prev_size << " bytes"<<endl;
		if(trash.isFile){
			cout<<"Type: File"<<endl;
		}else{
			cout<<"Type: Folder"<<endl;
		}
		cout<<"Previous path: "<<bin.front().prev_path<<endl;
	}else{//if bin is empty
		cout<<"Bin is empty!"<<endl;
	}
}
void emptybin(queue<Trash> &bin){//deletes all the contents of the bin queue
	while(!bin.empty()){
		bin.pop();
	}
	cout<<"Bin emptied successfully"<<endl;
}
void VFS::find(string name, inode* curr){//Searches position list and displays the path of the inode
	for(auto i: positionlist){
		if(name == i->name){
			cout<<"Found "<<name<<" in path: "<<pwd(i)<<endl;
		}
	}
}
void VFS::recover(inode* curr,queue<Trash> bin){//Function used to recover a node from bin
	bool foundpath = 0;//Variable used to flag whether the previous directory of the node is fond or not
	inode recover_inode = bin.back().node;
	string name = bin.back().prev_path;//name of the trashed inode
	vector<string> names;
	string temp;
	stringstream strStream(name);
	while(getline(strStream, temp,'/')){//if name is "" then set it to "/"
		if(temp == ""){
			temp = "/";
		}
		names.push_back(temp);//last name vector is the filename secondlast is the parent name
	}
	for(auto i : bin.back().prev_positionlist){
		positionlist.push_back(i);
	}
	names.pop_back();//Now the last name is the parent name
	for(auto i: positionlist){
		if(names.back() == i->name){//if parent found in list
			foundpath = 1;
			inode* recover = getposition(name);//The inode that is being recovered
			inode* prev_directory;//previous parent of inode
			if(names.back()=="/"){//if the parent is the root
			prev_directory = getRoot();
			}else{//if it is not the root
			prev_directory = getchild(names.back(),curr);
			}
			recover->setparent(prev_directory);//sets childs parent back to its previous parent
			prev_directory->addchild(recover_inode);//add child back to parent's children vector
			break;//leave the loop
		}
	}if(foundpath == 0){//if the previous directory is not found
		cout<<"Error: Previous parent directory does not exist"<<endl;
	}
}
void VFS::Touch(string file_name,int size,string date){//Function is used only in the loaddata function to create file inodes that do not have a parent link.
	inode* file = new inode(true, file_name, size, date,nullptr);
	positionlist.push_back(file);//adds new inode to positionlist
}
void VFS::Mkdir(string file_name,string date){
	if(file_name == "/"){//if the name is the root
	root->date = date;//set the date of the root to the one in the data file
	inode* folder = new inode(false, "/", 0 , date,nullptr);//create a inode with the roots details
	positionlist.push_back(folder);//adds the root to the positionlist
	}else{
	inode* folder = new inode(false, file_name, 10, date,nullptr);//Function is used only in the loaddata function to create folder inodes that do not have a parent link.
	positionlist.push_back(folder);}//adds new inode to positionlist
}
inode* VFS::getposition(string name){//Function is used to retreive the inode of the given the name
	string temp;
	if(name[0] == '/'){ //if the name is a path, extract the name from it
		stringstream strStream(name);
		while(getline(strStream, temp,'/')){}
		getline(strStream,temp);
		name = temp;
	}
	for(auto i: positionlist){ //Iterate through the positon till a inode with the same name is found and return it
		if(i->name == name){
			return i;
		}
	}
	cout<<"Error: Specified file does not exist"<<endl;
	throw Error();
}
void VFS::loaddata(string filename){ //Function used to load data at the start of the program
	string temp;
	ifstream inFile;
	string name, size, date;
	vector<string> vector_name;
    inFile.open(filename);
    if (!inFile.is_open()) {//Error if file fails to open
        cout << "Error: File could not be opened";
        return;
    }
    string lineContents;
    while (getline(inFile, lineContents)){//iterate through each line of the file
        stringstream strStream(lineContents);
        getline(strStream, name, ',');
        getline(strStream, size, ',');
        getline(strStream, date);
		if(name == "/"){//If the name is a root
			Mkdir(name,date);//create directory with root details
			continue;
		}
		stringstream strStream1(name);
		while(getline(strStream1, temp,'/')){//extract filename from path
			vector_name.push_back(temp);
		}
		getline(strStream1,temp);
		vector_name.push_back(temp);
		bool isfile = 0;
		for(char i: vector_name.back()){//Check if the name is a file
			if(i == '.'){
				isfile = 1;
				Touch(vector_name.back(), stoi(size),date);//if a name is a file create a file inode
			}
		}if(isfile == 0){
			Mkdir(vector_name.back(),date);//if a name is a folder create a folder inode
		}
	}
	inFile.close();
	ifstream inFile2;//reopen the same file, this is to reset the cursor of the file back to the beginning
	inFile2.open(filename);
	getline(inFile2, lineContents);
    while (getline(inFile2, lineContents)){//iterate through each line of the file
        stringstream strStream2(lineContents);
        getline(strStream2, name, ',');
        getline(strStream2, size, ',');
        getline(strStream2, date);
		stringstream strStream3(name);
		while(getline(strStream3, temp,'/')){//extract filename from path
			vector_name.push_back(temp);
		}
		vector_name.push_back(temp);
		int isRoot = 0; //Variable used to check if its a root
		for(int i = 0;i<name.length();i++){ //if path only has one '/' that means the parent node is the root
			if(name[i] == '/'){
				isRoot++;
			}
		}if(isRoot == 1){ //It is a root
		getposition(vector_name[vector_name.size()-1])->setparent(getRoot()); //vector_name.size()-1 is the index of the file. Set inodes parent as root
		getRoot()->addchild(*getposition(vector_name[vector_name.size()-1])); //add current inode to as a child of the root
		}else{
		inode* child = getposition(vector_name[vector_name.size()-1]); //vector_name.size()-1 is the index of the file. set it as a child inode for easier handling
		vector_name.pop_back();//delete the back inode so the next one is parent
		inode* parent = getposition(vector_name[vector_name.size()-2]); //vector_name.size()-2 is the index of the parent. set it as a parent inode for easier handling
		child->setparent(parent); //set a parent for the child
		parent->addchild(*child); //add child to parent children vector
		}
	}
	cout<<"Data loaded Successfully..."<<endl;
}
void VFS::dumpdata( ofstream &outFile, inode* curr){ //Function used to save data to a text file
	for(auto i: positionlist){ //iterate through positionlist
		outFile << pwd(i) << "," << i->size << "," << i->date << endl; //output inode details to outfile
	}
	return;
}
void VFS::Exit(string filename){ //Used to exit the program
	ofstream outFile;
	outFile.open(filename);
	if (!outFile.is_open()){ //error if file couldnt be opened and cancel the exit function
        cout << "Error: File could not be opened";
        return;
    }
	dumpdata(outFile,getRoot()); //save data to file before exiting
	cout<<"Data has been saved..."<<endl;
	cout<<"Exiting..."<<endl;
	exit(1); //Exit program
}
void VFS::help(){ //output the list of commands
    cout<<"List of available Commands:"<<endl
		<<"help                                    : Prints menu of commands"<<endl
		<<"pwd                                     : Prints the path of current inode"<<endl
		<<"realpath <filename>                     : Prints the absolute/full path of a given filename of a file within the current directory"<<endl
		<<"ls                                      : Prints the children of the current inode"<<endl
		<<"mkdir <foldername>                      : Creates a folder under the current folder"<<endl
		<<"touch <filename> <size>                 : Creates a file under the current inode location"<<endl
		<<"cd                                      : Change current directory"<<endl
		<<"find <foldername> || <filename>         : Returns the path of the file (or the folder) if it exists"<<endl
		<<"mv <filename> <foldername>              : Moves a file located under the current inode location, to the specified folder path"<<endl
		<<"rm <filename> || <foldername>           : Removes the specified folder or file and puts it in a Queue of MAXBIN=10"<<endl
		<<"size <foldername> || <filename>         : Returns the total size of the folder"<<endl
        <<"emptybin                                : Returns the capacity of vector"<<endl
		<<"showbin                                 : Tests if the vector container is empty"<<endl
		<<"recover                                 : Reinstates the oldest inode back from the bin to its original position in the tree"<<endl
		<<"exit/quit                               : Exit the Program"<<endl;
}
