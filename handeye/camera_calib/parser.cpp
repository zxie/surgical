#include<iostream>
#include<string.h>
#include<fstream>
#include<sstream>
#include<stdio.h>
#include<stdlib.h>
using namespace std;

void usage(const char *s){
  std::cout << "usage:\n" << s << " -n NumberPoses -p posFile < resultsFile.log > matlabFile.m" << std::endl;
}

int main(int argc, char **argv){
  if (argc==1){
    usage(argv[0]);
    return 0;
  }
  int counts;
  string posFile;
  for (int i=1; i<argc; i++){
    if (strcmp(argv[i], "-n")==0){
      counts = atoi(argv[i+1]);i++;
    }
    else if (strcmp(argv[i], "-p")==0) {
      posFile = argv[i+1];i++;
    }
    else{
      usage(argv[0]);
      return 0;
    }
  }

  string s;
  getline(cin,s);
  while(s.substr(0,4)!="----"){ //busco la linea con los guiones
    getline(cin,s);
  }
  
  int in=1;
  double x;
  ifstream fpos;
  fpos.open(posFile.c_str());
  if(!fpos.good()){
      cerr<<"error al abrir el archivo: "<<posFile.c_str()<<endl;
      return 0;
  }
  while(in < counts+1){
        cout<<"Hgrid2cam(:,:,"<<in<<") = [";
        for(int i=0;i<3;i++){
            for(int j=0;j<4;j++){
              if (cin>>x)
                cout<<" "<<x;
              else
                return 0;
            }
            cout<<";";
        }
        cout<<" 0 0 0 1];"<<endl;
        cout<<"Hcam2grid(:,:,"<<in<<") = inv(Hgrid2cam(:,:,"<<in<<"));"<<endl;;
        double f[6];  
        for(int i=0;i<6;i++){
            fpos>>f[i];
        }
        cout<<"Hmarker2world(:,:,"<<in<<") = [ RotationMatrix("<<f[3]<<","<<f[4]<<","<<f[5]<<") ["<<f[0]<<";"<<f[1]<<";"<<f[2]<<"]; 0 0 0 1;];"<<endl;

        cout<<"Hworld2marker(:,:,"<<in<<") = inv(Hmarker2world(:,:,"<<in<<"));"<<endl;;

        in++;
    }
    fpos.close();
    return 0;
}


