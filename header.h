#ifndef FATCHINSTRUCTION_H
#define FATCHINSTRUCTION_H
#ifndef DECODEINSTRUCTION_H
#define DECODEINSTRUCTION_H
#ifndef LOADINSTRUCTION_H
#define LOADINSTRUCTION_H
#ifndef REPLACEVALUE_H
#define REPLACEVALUE_H
#include<time.h>

int load(char *filenm,char *resource_name, int lengther)    //loading the instruction
{
	FILE *fp = fopen(filenm, "r");  //selecting the instruction from the file
	char ch[2],*tmp;
	int insSize=0;
	ch[0] ='0' ;
	int looper;
	int var_length;
	int success=0;
	int answer;

	while (ch[0] != EOF)
	{		
		while(ch[0]!='\n'){
			ch[0] = getc(fp);	
			insSize++;	
		}
		fseek(fp,-(insSize),SEEK_CUR);
		tmp = (char *)malloc(insSize*sizeof(char));
		fread(tmp,insSize,sizeof(char),fp);
		int length=sizeof(tmp);
		int i = 0;

		while(tmp[i]!=32)
		{	
			i++;			
		}
		var_length=i;
		
		success=0;
		for(i=0;i<lengther;i++){
			if(resource_name[i]==tmp[i]){
				success++;		
			}	
		}

	
		if(success==lengther && success==var_length){
			answer=atoi(&tmp[var_length+1]);	
		}

		ch[0] = getc(fp);
		insSize=1;
		
	}
		fclose(fp);
  return answer;
}
#endif /* LOADINSTRUCTION_H */

void replaceVal(char *instruction, int value,int processID) // This will update the value in the file 
{
	int len = strlen(instruction);  //finding the instruction length
        FILE *re;
        if(processID == 1){     //Checking the thread id
            re = fopen("proc/1/variables.txt","r"); //As per the thread id select the variables.
        }
        else{
            re = fopen("proc/2/variables.txt","r");
        }

	
	char c=getc(re);
	int count=1;
	while(c!=EOF)
	{c=getc(re);count++;
	}
	rewind(re);
	char *buff = (char *)malloc(count*sizeof(char));    //allocating the memory to the buffer
	int i=1,j=0,k=0,l=0;
	
	for(i = 0;i<count;i++)  //Checking which variable need to update 
	buff[i]=getc(re);
	fclose(re);
	
	for(i=0,j=0;j<count-1;j++)
	{
		
		if(buff[j]==instruction[i])
		{
			k++;i++;
			if(k==len && buff[j+1]==' ')
			{
				j-=(k-1);
				while(buff[j]!='\n' )
				{
					buff[j]='*';        //putting * in the place of current value
					j++;
				}
				buff[j]='*';
				
			}
		}
		else
		{
			while(buff[j]!='\n' )
			j++;
			i=0;
			k=0;
		}
	}
	
	if(processID == 1){ //open the variables file to update the variable value
            re = fopen("proc/1/variables.txt","w");
        }
        else{
            re = fopen("proc/2/variables.txt","w");
        }
	
	i=0;
	for(i=0;i<len;i++)
	fputc(instruction[i],re);
	fprintf(re," %d\n",value);
	
	i=0;
	c=buff[i];
	while(c!=EOF)
	{
		while(c=='*')   //Putting the variable value instated of *
		{
			i++;
			c=buff[i];
		}
		fputc(c,re);
		i++;
		c=buff[i];
	}
	fclose(re);
	
	
}


#endif /* REPALCEVALUE_H */

void decode(void *ins, int processID, char *fileIn, int fileline){ //Decoding the instruction
                                    //Which operation need to be done
                                    //Which variable need to update
	
	char *instruction = (char *)ins;
	int i=0;
	time_t t;
	while(instruction[i]!='\n'){
	i++;
	}

	instruction[i]='\0';
	int size=strlen(instruction);
	
	int j;
	float variable_1=0, variable_2=0, answer=0;
	i=0;
	while(instruction[i]!=32){
	i++;
	}
	FILE *fptr;
	int insCode=load("inscode.txt",&instruction[0],i);  //Decoding the operation
        switch(insCode){
	
	case 1:     //For addition
		i++;
		j=i;
                while(instruction[i]!=' '){
                i++;
                }
                if(processID == 1){
                    variable_1 = load("proc/1/variables.txt",&instruction[j],i-j);//loading the first variable
                }
                else{
                    variable_1 = load("proc/2/variables.txt",&instruction[j],i-j);
                }
                    
		i++;
		j=i;
		while(instruction[i]!=32){
		i++;
		}
                if(processID == 1){
                    variable_2 = load("proc/1/variables.txt",&instruction[j],i-j);    //loading the second variable
                }
                else{
                    variable_2 = load("proc/2/variables.txt",&instruction[j],i-j);    
                }
//		variable_2 = load("variables.txt",&instruction[j],i-j);
		i++;
		j=i;
		while(instruction[i]!=32 && i<strlen(instruction)){
		i++;
		}
		answer = variable_1+variable_2;
                //pthread_t logThreadId;
                //struct logparameter l;
                //l.answer = answer;
                //l.ptr = fptr;
                //l.pid = processID;
                
                //pthread_create(&logThreadId,NULL,logThread,&l);
                if(processID == 1){
                    fptr = fopen("proc/1/rr_log.txt","a+");
                }
                else{
                    fptr = fopen("proc/2/rr_log.txt","a+");
                }
                
                if(fptr==NULL)
                {
                        printf("Error");
                        exit(1);
                }
                else
                {
                        time(&t);
                        fprintf(fptr,"\nfilename: %s \t fileline: %d \t %s Thread ID = %d i + j = %f\n",fileIn, fileline, ctime(&t), processID-1, answer);
                }
                fclose(fptr);
                
		//here we need to write value of answer inplace of "k" in resource.txt
		replaceVal(&instruction[j],answer,processID);
		break;
        case 2: //For subtraction
            i++;
            j=i;
            while(instruction[i]!=' '){
            i++;
            }
            if(processID == 1){
                variable_1 = load("proc/1/variables.txt",&instruction[j],i-j);//loading the first variable
            }else{
                variable_1 = load("proc/2/variables.txt",&instruction[j],i-j);
            }
            
            i++;
            j=i;
            while(instruction[i]!=32 ){
            i++;
            }
            if(processID == 1){
                variable_2 = load("proc/1/variables.txt",&instruction[j],i-j); //loading the second variable
            }else{
                variable_2 = load("proc/2/variables.txt",&instruction[j],i-j);
            }
            
            
            i++;
            j=i;
            while(instruction[i]!=32 && i<strlen(instruction)){
            i++;
            }
            answer = variable_1-variable_2;   //Doing subtraction
            if(processID == 1){
                fptr=fopen("proc/1/rr_log.txt","a+");
            }else{
                fptr=fopen("proc/2/rr_log.txt","a+");
            }
            
            if(fptr==NULL)
            {
                    printf("Error");
                    exit(1);
            }
            else
            {
                   time(&t);
                   fprintf(fptr,"\nfilename: %s \t fileline: %d \t %s Thread ID = %d a - b = %f\n",fileIn, fileline, ctime(&t), processID-1, answer);
            }
            fclose(fptr);
            replaceVal(&instruction[j],answer,processID);//Replacing the value of the current variable.
            break;
        case 3: //For Multiplication
            i++;
            j=i;
            while(instruction[i]!=' '){
            i++;
            }
            if(processID == 1){
                variable_1 = load("proc/1/variables.txt",&instruction[j],i-j); //loading the first variable
            }else{
                variable_1 = load("proc/2/variables.txt",&instruction[j],i-j);    //loading the first variable
            }
            
            i++;
            j=i;
            while(instruction[i]!=32 ){
            i++;
            }
            if(processID == 1){
                variable_2 = load("proc/1/variables.txt",&instruction[j],i-j);    //loading the second variable
            }else{
                variable_2 = load("proc/2/variables.txt",&instruction[j],i-j);
            }
            
            i++;
            j=i;
            while(instruction[i]!=32 && i<strlen(instruction)){
            i++;
            }
            answer = variable_1*variable_2;   //Doing proper operation
            if(processID == 1){
                fptr=fopen("proc/1/rr_log.txt","a+");
            }else{
                fptr=fopen("proc/2/rr_log.txt","a+");
            }
            
            if(fptr==NULL)
            {
                    printf("Error");
                    exit(1);
            }
            else
            {
                    time(&t);
                    fprintf(fptr,"\nfilename: %s \t fileline: %d \t %s Thread ID = %d c * d = %f\n",fileIn, fileline, ctime(&t), processID-1, answer);
            }
            fclose(fptr);
            replaceVal(&instruction[j],answer,processID);//Replacing the value of the current variable.

            break;
        case 4: //For division
            i++;
            j=i;
            while(instruction[i]!=' '){
            i++;
            }
            if(processID == 1){
                variable_1 = load("proc/1/variables.txt",&instruction[j],i-j); //loading the first variable from the file
            }else{
                variable_1 = load("proc/2/variables.txt",&instruction[j],i-j);
            }
            
            i++;
            j=i;
            while(instruction[i]!=32 ){
            i++;
            }
            if(processID == 1){
                variable_2 = load("proc/1/variables.txt",&instruction[j],i-j);//loading the second variable
            }else{
                variable_2 = load("proc/2/variables.txt",&instruction[j],i-j);
            }
            i++;
            j=i;
            while(instruction[i]!=32 && i<strlen(instruction)){
            i++;
            }
            answer = (float)variable_1/variable_2;
            if(processID == 1){
                fptr=fopen("proc/1/rr_log.txt","a+");
            }else{
                fptr=fopen("proc/2/rr_log.txt","a+");
            }
            if(fptr==NULL)
            {
                    printf("Error");
                    exit(1);
            }
            else
            {
                    time(&t);
                    fprintf(fptr,"\nfilename: %s \t fileline: %d \t %s Thread ID = %d x / y = %f\n",fileIn, fileline, ctime(&t), processID-1, answer);
            }
            fclose(fptr);
            replaceVal(&instruction[j],answer,processID); //Replacing the value of the current variable.

            break;

	}

}

#endif /* DECODEINSTRUCTION_H */

int fetch(char *fileIn,int processID)   //fetching the instruction
{
	FILE *fp= fopen(fileIn,"r");    //Open the file for reading
	FILE *fptr;
	char ch='0',*tmp;
	int insSize=0;
	int fileline =0;
	pthread_t insExe;int insStatus;
	
	while (ch != EOF)
	{	
		while(ch!='\n'){
			ch = getc(fp);	
			insSize++;	
		}
		fseek(fp,-(insSize),SEEK_CUR);
		tmp = (char *)malloc(insSize*sizeof(char));
		fileline++;
		fread(tmp,insSize,sizeof(char),fp);
		decode((void *)tmp,processID,fileIn,fileline);
		ch = getc(fp);
		insSize=1;
		
	}
	fclose(fp);
	
  return 0;
}

#endif /* FATCHINSTRUCTION_H */
