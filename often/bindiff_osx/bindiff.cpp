#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;
#define ROWSCOUNTEACHLINE 30
unsigned int GetLineNumber(const char * Str,unsigned int);
bool OutputTwoFiles(FILE * file1,FILE * file2,unsigned int Lines);
bool OutputOneFiles(FILE * file,unsigned int Lines);
void OutputString(char * Str,unsigned int Rows,bool bWithEndl);

int main (int argc, const char * argv[]) {
    // insert code here...
    FILE * pFile=NULL;
    FILE * pFileTarget=NULL;
    unsigned int ShowDiffLineCount=50;
    bool bOutputOneFile=true;
    
    cout<<"BinDiff tool for Mac OS"<<endl;
    cout<<"     Written by Horky.Chen,2006/2/14"<<endl;
    
    if(argc<2)
      {
         cout<<" *********************************************"<<endl;
         cout<< "Usage:" <<endl;
         cout<< " bindiff filename1 [filename2] [options]" <<endl;
         cout<< " Options: " <<endl;
         cout<< "    -nNUMBER  Display top specific differential lines!"<<endl;
         cout<< "       For example: -n50"<<endl;
         cout<<" ********************************************"<<endl;
         return 0;
      }
    else if(argc==3)
    {
        if((argv[2][0]=='-') && ((argv[2][1]=='n') || (argv[2][1]=='N')))
           bOutputOneFile=true;
        ShowDiffLineCount = GetLineNumber(argv[2],2);
        if(ShowDiffLineCount == 0)
          return 1;
    }
    else if(argc==2)
        bOutputOneFile=true;
    else if(argc>3)
      { 
        bOutputOneFile=false;
        if( (argv[3][1]=='n') || (argv[3][1]=='N'))
        {
            ShowDiffLineCount = GetLineNumber(argv[3],2);
            if(ShowDiffLineCount == 0)
                return 1;
         }
      }
     
    cout<<" Display lines count is "<<ShowDiffLineCount<<endl;
    
    pFile = fopen(argv[1],"rb");
    if(!bOutputOneFile)
       pFileTarget = fopen(argv[2],"rb");
    
    if(pFile == NULL)
    {
      cout <<"The File " << argv[1] <<" no exists!" <<endl;
      return 2;
    }
    
    if( !bOutputOneFile && (pFileTarget == NULL))
     {
       cout <<" The File " << argv[2] <<" not exists!" <<endl;
     }
    
    if(bOutputOneFile) 
        OutputOneFiles(pFile,ShowDiffLineCount);
    else
        OutputTwoFiles(pFile,pFileTarget,ShowDiffLineCount);
    
    cout<<"*********** Finish of file *************"<<endl;
   if(pFile!=NULL) 
        fclose(pFile);
   if(pFileTarget!=NULL) 
        fclose(pFileTarget);
    return 0;
}


unsigned int GetLineNumber(const char * Str,unsigned int count)
{
     unsigned int i = count;
     unsigned int result =0;
     
     cout<<" Specify the number of lines to display! "<<endl;
     while(1)
     {
       if((Str[i]<'0')||(Str[i]>'9'))
         break;
        result=result*10+Str[i]-48;
        //cout<<"   Result is " <<result<<endl;
        i++;
        if((i-count)>5)
          break;
     }
    
     return result;
}

bool OutputOneFiles(FILE * file,unsigned int Lines)
{
    char Str[255]={0};
    unsigned int LineNo=0;
    char TempStr[20];
    while(fread((void *)Str,ROWSCOUNTEACHLINE,1,file)!=NULL)
      {
        Lines--;
        LineNo++;
        sprintf(TempStr,"%5d : ",LineNo);
        cout<<TempStr;
        OutputString(Str,ROWSCOUNTEACHLINE,true);
        if(Lines==0)
          break;
        memset(Str,0,255);
      }
    return true;
}

bool OutputTwoFiles(FILE * file1,FILE * file2,unsigned int Lines)
{
    char Str1[255]={0};
    char Str2[255]={0};
    unsigned int LineNo=0;
    char TempStr[20];
    int ReadCount1,ReadCount2;
    while(true)
      {
        ReadCount1 = fread((void *)Str1,ROWSCOUNTEACHLINE,1,file1);
        ReadCount2 = fread((void *)Str2,ROWSCOUNTEACHLINE,1,file2);
        if(ReadCount1==NULL && ReadCount2==NULL)
          break;
        Lines--;
        LineNo++;
        
        //output the context of each files
        sprintf(TempStr,"%5d : ",LineNo);
        cout<<TempStr;
        OutputString(Str1,ROWSCOUNTEACHLINE/2,false);
        cout<<"  |  ";
        OutputString(Str2,ROWSCOUNTEACHLINE/2,true);
        
        if(Lines==0)
          break;
        memset(Str1,0,255);
        memset(Str2,0,255);
      }
      
  return true;
}

void OutputString(char * Str,unsigned int Rows,bool bWithEndl)
{
  char TempStr[3]={0};
  char OutStr[255]="";
  for(unsigned int i=0;i<Rows;i++)
  {
    sprintf(TempStr,"%02X ",(unsigned char)Str[i]);
    strcat(OutStr,TempStr);
    memset(TempStr,0,3);
   }
  cout<<OutStr;
  
  if(bWithEndl)
     cout<<endl; 
}