#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include<dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;
string current_dir;
struct termios orig_termios;
//we need stack data structure to keep track of recently visited directories
stack<string>last_visited;
stack<string>next_visit;
//we will have 4 vectors for files,folders,allstuff,allcheck
vector<string>files,folders,allstuff;
vector<int>allcheck;

static struct termios term, oterm;

void Normalmode(string path);
void Commandmode(int start,int end,int f);

//to return home directory
string homedir(){
struct passwd *pw = getpwuid(getuid());

const char *homedir = pw->pw_dir;
string s(homedir);
s+='/';
return s;
}

static int getch(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(ICRNL | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

//function to set terminal
void sett(){
//first we set the terminal to full screen mode
cout << "\e[3;0;0t";
cout << "\e[8;55;205t";
}

//function to return current working directory
string cwd(){
char cwd[256];
getcwd(cwd, 256);
string cwd_str = string(cwd);
cwd_str+='/';
return cwd_str;
}

//to print files and folders of directory
void print(int start, int end){
cout << "\033[2J";     //clearing the terminal
cout << "\033[0;0H";   //setting cursor to top left


cout.width(38);
cout<<left<<"\033[1m\033[36mNAME\033[0m";
cout.width(33);
cout<<left<<"\033[1m\033[36mSIZE\033[0m";
cout.width(28);
cout<<left<<"\033[1m\033[36mGROUP\033[0m";
cout.width(28);
cout<<left<<"\033[1m\033[36mUSER\033[0m";
cout.width(33);
cout<<left<<"\033[1m\033[36mPERMISSIONS\033[0m";
cout<<left<<"\033[1m\033[36mLAST MODIFIED\033[0m";
cout<<endl;
cout<<"\033[1m\033[36m-------------------------------------------------------------------------------------------------------------------------------\033[0m";
cout<<endl;

struct stat sb;
struct group *grp;
struct passwd *pwd;
for(int i=start;i<=end;i++){
const char* pathchar = (allstuff[i]).c_str();
if( stat(pathchar, &sb) == 0 ){
string name = allstuff[i].substr(allstuff[i].find_last_of("/\\") + 1);
string basename="";
for(int i=0;i<name.length() && i<20;i++){
if(i<=16)basename+=name[i];
else basename+=".";
}
cout.width(25);
cout<<left<<basename;


float size = (sb.st_size/1024.0);
stringstream ss;
ss << fixed << std::setprecision(2) << size;
std::string s1 = ss.str();
cout << std::left << setw(20);
string sizef = s1 + " KB";
cout << sizef;


grp = getgrgid(sb.st_gid);
pwd = getpwuid(sb.st_uid);
cout<<left<<setw(15)<<grp->gr_name;
cout<<left<<setw(15)<<pwd->pw_name;

string p = (S_ISDIR(sb.st_mode)) ? "d" : "-";
p+= (sb.st_mode & S_IRUSR) ? "r" : "-";
p +=(sb.st_mode & S_IWUSR) ? "w" : "-";
p +=(sb.st_mode & S_IXUSR) ? "x" : "-";
p +=(sb.st_mode & S_IRGRP) ? "r" : "-";
p +=(sb.st_mode & S_IWGRP) ? "w" : "-";
p +=(sb.st_mode & S_IXGRP) ? "x" : "-";
p+=(sb.st_mode & S_IROTH) ? "r" : "-";
p +=(sb.st_mode & S_IWOTH) ? "w" : "-";
p +=(sb.st_mode & S_IXOTH) ? "x" : "-";
cout<<left<<setw(20)<<p;

char *time;
time=ctime(&sb.st_mtime);
string t(time);
cout<<t;
cout<<endl;

}
}
cout << "\033[1m\033[36mCurrent Directory --> \033[1m\033[36m"<<current_dir;
}

//to update all the lists of files and folders
void updatelist(string path){
files.clear();
folders.clear();
allstuff.clear();
allcheck.clear();
DIR *dr;
struct dirent *en;
const char* pathchar = (path).c_str();
dr = opendir(pathchar);
while((en=readdir(dr))!=NULL){
string fpath=path+en->d_name;
const char* fpathchar = (fpath).c_str();
struct stat sb;
if (stat(fpathchar, &sb) == -1) {
perror("stat");
exit(EXIT_FAILURE);
}
 switch (sb.st_mode & S_IFMT){
 case S_IFDIR:{
 folders.push_back(fpath);
 break;
 }
 case S_IFREG:{
 files.push_back(fpath);
 break;
 }
 }
}
sort(files.begin(),files.end());
sort(folders.begin(),folders.end());
for(int i=0;i<folders.size();i++){
allstuff.push_back(folders[i]);
allcheck.push_back(1);//1 for folder
}
for(int i=0;i<files.size();i++){
allstuff.push_back(files[i]);
allcheck.push_back(0);//1 for folder
}
closedir(dr);
}

string key(){
char c=getch();//read first char of input
if(c=='\033'){
//control sequence
c=getch();
if(c=='['){
c=getch();
switch(c){
case 'A':{
return "U";
break;
}
case 'B':{
return "D";
break;
}
case 'C':{
return "R";
break;
}
case 'D':{
return "L";
break;
}
}
}
}
else{
switch(c){
case 'h':{
return "H";
break;
}
case 'q':{

return "Q";
break;
}
case 10:{
return "ENTER";
break;
}
case 59:{
return "COLON";
break;
}
case 58:{
return "COLON";
break;
}
case 127:{
return "BACKSPACE";
break;
}
}
}
string ret(1,c);
return ret;
}


void rawmode(int start, int end, int f){
//enableRawMode();
start*=2;
start+=3;
end*=2;
end+=3;
//end+=2;
cout << "\033[3;0H";//placing the cursor position
if(f==1){
for(int i=start;i<end;i++){
cout << "\x1b[1B";
//cout << "\x1b[1B";
}
}
int n=allstuff.size()-1;//total no of lines
n*=2;
n+=3;
//int s=2;//start of the line
int c=start;//current cursor position
if(f==1)c=end;
//while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
while(1){
string inp=key();
if(inp=="Q"){
//quit
cout << "\033[2J";     //clearing the terminal
cout << "\033[0;0H";   //setting the cursor to top-left
exit(0);
}
else if(inp=="U" && c>start){
c-=2;
cout << "\x1b[1A";
cout << "\x1b[1A";
}
else if(inp=="U" && c>3){
//upward overflow
if(c!=3){
int s=start-3;
s=s/2;
int e=end-3;
e=e/2;
print(s-1,e-1);
rawmode(s-1,e-1,0);
}
}
else if(inp=="D" && c<end){
c+=2;;
cout << "\x1b[1B";
cout << "\x1b[1B";
}
else if(inp=="D" && c>=end){
//downward overflow
if(c!=n){
int s=start-3;
s=s/2;
int e=end-3;
e=e/2;
print(s+1,e+1);
rawmode(s+1,e+1,1);
}
}
else if(inp=="ENTER"){
int in=c-3;
in=in/2;
//in=in-1;
current_dir=allstuff[in];
if(allcheck[in]==0){
//files
char path[1024];
strcpy(path, allstuff[in].c_str());//copying the string path to char array

int l=allstuff[in].length();
string p=allstuff[in];
if(p[l-3]!='o' && p[l-2]!='u' && p[l-1]!='t'){

int fd = open(path,O_WRONLY);//open file descriptor in write only mode
dup2(fd, 2);//duplicate file descriptor
close(fd);
pid_t pid = fork();
if(pid == 0)//child process
{
execlp("open","open",path,NULL);
exit(0);
}
}
}
else{
//folder
int in=c-3;
in=in/2;
string path=allstuff[in];
path+="/";
last_visited.push(path);
Normalmode(path);
}
}
else if(inp=="H"){
string path=homedir();
current_dir=path;
last_visited.push(path);
current_dir=path;
Normalmode(path);
}
else if(inp=="BACKSPACE"){

string path=last_visited.top();
if(path!=homedir()){
next_visit.push(path);
path = path.substr(0, path.find_last_of("/\\"));
path = path.substr(0, path.find_last_of("/\\"));
path+='/';
last_visited.push(path);
current_dir=path;
Normalmode(path);
}
}
else if(inp=="L"){
//remove the top of stack
if(last_visited.size()>1){
string path=last_visited.top();
next_visit.push(path);
last_visited.pop();
path=last_visited.top();
current_dir=path;
Normalmode(path);
}
}
else if(inp=="R"){
if(next_visit.size()>0){
string path=next_visit.top();
next_visit.pop();
last_visited.push(path);
current_dir=path;
Normalmode(path);
}
}
else if(inp=="COLON"){
Commandmode(start,end,f);
}
}
}

//Normal mode
void Normalmode(string path){
//path="/home/anuja/";
sett();//set the terminal to max size
updatelist(path);//to update all the files and folders list for current operation
int start=0;
int end=10;
if(allstuff.size()<11)end=allstuff.size()-1;
print(start,end);//will print the files and folders with limit 0-10
rawmode(start,end,0);
}

//key function for command mode
string key2(){
char c;
c=getch();
if(c==27)return "ESC";
if(c==10)return "ENTER";
if(c==127)return "BACKSPACE";
string ret(1,c);
return ret;
}

//to check if the file.drectory exists or not
char check_dir_file(string str)
{
    struct stat temp_stat;
    //cout<<"\nchecking "<<str<<endl;
    if (stat(str.c_str(), &temp_stat) == -1) {
        //response="error opening the file/directory";
        return 'n';
    }

    if (S_ISDIR(temp_stat.st_mode))
    {
        return 'd';

    }
    else
    {
        return 'f';
    }
}


int copy_file(string source, string dest){
if(dest[dest.length()-1]!='/')dest+='/';
string name=source.substr(source.find_last_of('/')+1);
dest+=name;
int in3=creat(dest.c_str(),O_WRONLY);
struct stat st;
stat(source.c_str(),&st);
chmod(dest.c_str(),st.st_mode);
int from=open(source.c_str(),O_RDONLY);
int to=open(dest.c_str(),O_WRONLY|O_CREAT,0);
if(from<0 || to<0){
return -1;
}
int n;
char *filebuff=new char[n];
while((n=(read(from,filebuff,sizeof(filebuff)))) > 0)
{
write(to,filebuff,n);
}
close(from);
close(to);
return 0;
}


int copy_dir(string source,string destination)
{
struct stat st,temp;
lstat(source.c_str(), &st);

/*string namedir=source.substr(source.find_last_of('/')+1);
destination+="/";
destination+=namedir;
struct stat st2;
stat(source.c_str(),&st2);
int in3=creat(destination.c_str(),O_WRONLY);
chmod(destination.c_str(),st2.st_mode);*/

int f=mkdir(destination.c_str(), st.st_mode);
DIR *dr;
struct dirent *d;
vector<string>files;
dr = opendir(source.c_str());
if (dr == nullptr)return -1;
for (d = readdir(dr); d != nullptr; d = readdir(dr)) files.push_back(string(d->d_name));
closedir(dr);
for (int i = 0; i < files.size(); i++) {
if (files[i] == "." || files[i] =="..")continue;
string name = files[i];
string temppath_src = source + "/" + name;
string temppath_dest = destination + "/" + name;
if (lstat(temppath_src.c_str(), &temp) == -1) continue;
if (S_ISDIR(temp.st_mode))
{
copy_dir(temppath_src, temppath_dest);
}
else if (S_ISREG(temp.st_mode))
{
copy_file(temppath_src, destination);
}
}
return 0;
}

//copy_function
int copy_fun(string str){
//cout<<"0";
string source="",dest="";
str=str.substr(str.find(" ")+1);
source=str.substr(0,str.find(" "));
dest=str.substr(str.find(" ")+1);
char c=check_dir_file(source);
//cout<<"\n"<<source<<endl;
//cout<<dest<<endl;
//cout<<c;
if(c=='d'){
//source is directory
//cout<<"d ";
string namedir=source.substr(source.find_last_of('/')+1);
dest+="/";
dest+=namedir;
struct stat st2;
lstat(source.c_str(),&st2);
int f=mkdir(dest.c_str(), st2.st_mode);
cout<<f;
return copy_dir(source,dest);
//return 0;
}
else if(c=='f'){
//source is file
//cout<<"f";
return copy_file(source,dest);
//return f(source,dest);
}
//cout<<"n";
if(c=='n')return -1;
return 0;
}

int move_dir(string source, string dest){
DIR *dr;
struct stat temp;
struct dirent *d;
vector<string>files;
dr = opendir(source.c_str());
if (dr == nullptr)return -1;
for (d = readdir(dr); d != nullptr; d = readdir(dr)) files.push_back(string(d->d_name));
closedir(dr);
string root=source.substr(source.find_last_of("/\\")+1);
//struct stat st_tt;
//stat(source.c_str(),&st_tt);
string destt=dest+"/"+root;
//int f=mkdir(destt.c_str(),st_tt.st_mode);
for (int i = 0; i < files.size(); i++) {
if (files[i] == "." || files[i] =="..")continue;
string name = files[i];
string temppath_src = source + "/" + name;
string temppath_dest = dest + "/" +root+"/"+ name;
//cout<<temppath_src<<" "<<temppath_dest<<" "<<endl;
if (lstat(temppath_src.c_str(), &temp) == -1) continue;
if (check_dir_file(temppath_src)=='d')
{
//cout<<"directory";
struct stat st_t;
stat(temppath_src.c_str(),&st_t);
int f=mkdir(temppath_dest.c_str(),st_t.st_mode);
//cout<<temppath_src<<" "<<temppath_dest<<" "<<f<<endl;
//string destt=dest+"/"+r
move_dir(temppath_src, destt);
rmdir(temppath_src.c_str());
}
else if (S_ISREG(temp.st_mode))
{
int f=rename(temppath_src.c_str() ,temppath_dest.c_str());
//cout<<temppath_src<<" "<<temppath_dest<<" "<<f<<endl;
if(f==-1)return f;
}
}
rmdir(source.c_str());
return 0;
}

int move_fun(string str){
//cout<<"0";
string s=str;
string source="",dest="";
str=str.substr(str.find(" ")+1);
source=str.substr(0,str.find(" "));
dest=str.substr(str.find(" ")+1);
char c=check_dir_file(source);
//cout<<"\n"<<source<<endl;
//cout<<dest<<endl;
//cout<<c<<endl;
if(c=='f'){
//source is directory
//cout<<"d ";

int f= copy_fun(s);
if(f==-1)return -1;
//return delete_dir(source);
int fd = remove(source.c_str());
if(fd<0)return -1;
return 0;
//return 0;
}
else if(c=='d'){
struct stat st_tt;
string root=source.substr(source.find_last_of("/\\")+1);
//struct stat st_tt;
stat(source.c_str(),&st_tt);
string destt=dest+"/"+root;
int f=mkdir(destt.c_str(),st_tt.st_mode);
return move_dir(source,dest);
}
if(c=='n')return -1;
return 0;
}

int rename_fun(string str){
string s=str;
string source="",dest="";
str=str.substr(str.find(" ")+1);
source=str.substr(0,str.find(" "));
dest=str.substr(str.find(" ")+1);
int f=rename(source.c_str(),dest.c_str());
if(f<0)return -1;
return 0;
}

int create_file(string str){
string source="",dest="";
str=str.substr(str.find(" ")+1);
source=str.substr(0,str.find(" "));
dest=str.substr(str.find(" ")+1);
string path=dest+"/"+source;
int fd=open(path.c_str(),O_WRONLY|O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IXOTH);
if(fd<0)return -1;
return 0;
}

int create_dir(string str){
string source="",dest="";
str=str.substr(str.find(" ")+1);
source=str.substr(0,str.find(" "));
dest=str.substr(str.find(" ")+1);
string path=dest+"/"+source;
int fd=mkdir(path.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IXOTH);
if(fd<0)return -1;
return 0;
}

int delete_file(string str){
string source="";
source=str.substr(str.find(" ")+1);
int fd=remove(source.c_str());
if(fd<0)return -1;
return 0;
return 0;
}


int delete_dir(string path)
{
string src="";
src=path.substr(path.find(" ")+1);
struct stat s_fstat, d_fstat, temp_stat;
stat(src.c_str(),&s_fstat);
DIR* dr;
struct dirent* d;
vector<string>files;
dr=opendir(src.c_str());
if(dr== nullptr)return -1;
for(d= readdir(dr);d!= nullptr;d= readdir(dr))
{
files.push_back(string(d->d_name));
}
closedir(dr);
for(int i=0;i<files.size();i++)
{
if(files[i] == "." || files[i] == "..")continue;
string temppath_src = src+"/"+ files[i];
lstat(temppath_src.c_str(),&temp_stat);
if(S_ISDIR(temp_stat.st_mode)==1)
{
delete_dir(temppath_src);
}
else
{
delete_file(temppath_src);
}
}
rmdir(src.c_str());
return 0;
}

int goto_fun(string str){
if(str[str.length()-1]!='/')str=str+"/";
string source="";
str=str.substr(str.find(" ")+1);
current_dir=str;
last_visited.push(current_dir);
Normalmode(current_dir);
return 0;
}

int search_fun(string str, string directory)
{
if(directory[directory.length()-1]!='/')directory+="/";
string filename="";
filename=str.substr(str.find(" ")+1);
struct stat temp_stat;
DIR* dr;
struct dirent* di;
dr=opendir(directory.c_str());
if(dr== nullptr)
{
return -1;
}
for(di= readdir(dr);di!= nullptr;di= readdir(dr))
{
string temp_dir =string(di->d_name);
//cout<<temp_dir<<" "<<filename<<endl;
if(temp_dir == "." || temp_dir == "..")
{
continue;
}
string temppath_src = directory+"/"+ string(di->d_name);
if(lstat(temppath_src.c_str(),&temp_stat)==-1)continue;
if(S_ISDIR(temp_stat.st_mode))
{
if(temp_dir.compare(filename)==0)
{
return 0;
}
else if(search_fun(filename,temppath_src)==0)
{
return 0;
}
}
else
{
if(temp_dir.compare(filename)==0)
{
return 0;
}
}
}
closedir(dr);
return -1;
}


//Command Mode
void Commandmode(int start,int end,int f){
cout << "\033[28;20H";
cout << "\033[1m\033[36mWelcome to Command Mode Anuja :) \033[1m\033[36m";
cout << "\033[1m\033[36m----------------------------------------------\n\033[1m\033[36m";
cout << "\033[31;20H" << "";

string inp="";
string command="";
string str="";
while(true){
inp=key2();
if(inp=="ESC"){
Normalmode(current_dir);
}
else if(inp=="BACKSPACE"){
str=str.substr(0,str.length()-1);
cout << "\33[2K";
cout << "\033[31;20H" << "";
cout<<str;
}
else if(inp=="ENTER"){
command=str.substr(0,str.find(" "));
//cout<<"\n"<<command;
int f=0;
//cout<<command;
if(command=="copy")f=copy_fun(str);
else if(command=="move")f=move_fun(str);
else if(command=="rename")f=rename_fun(str);
else if(command=="create_file")f=create_file(str);
else if(command=="create_dir")f=create_dir(str);
else if(command=="delete_dir")f=delete_dir(str);
else if(command=="delete_file")f=delete_file(str);
else if(command=="goto")f=goto_fun(str);
else if(command=="search")f=search_fun(str,current_dir);
else if(command=="quit"){
sett();
cout << "\033[2J";     //clearing the terminal
cout << "\033[0;0H";   //setting cursor to top left
exit(0);
}
else{
f=2;
}
//cout<<f;
str="";
cout << "\33[2K";//clear current line
cout << "\033[31;20H" << "";//position cursor
cout << "\033[35;20H" ;
if(f==0)cout <<"\033[33m"<<command<<" successfull !!"<<"\033[33m";//display status
else if(f==-1) cout <<"\033[33m"<<command<<" unsuccessfull !!"<<"\033[33m";//display status
else if(f==2)cout <<"\033[33m"<<" Command not found !!"<<"\033[33m";//display status
cout << "\033[31;20H" << "";//position cursor to where u wanna write command
cout<<"\033[1m\033[36m";
command="";
}
else{
str+=inp;
cout<<inp;
}
}
}


//main function
int main(){
current_dir=cwd();
last_visited.push(current_dir);
Normalmode(current_dir);

return 0;
}
