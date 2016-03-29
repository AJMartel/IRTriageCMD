#include <precomp.h>
#include "plc.h"
#include <plib.h>

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

int cmd_sethook(int id, void *pFunc);
int cmd_main (int argc, const TCHAR *argv[]);

#ifdef __cplusplus
}
#endif

char *argv[50];

int perlfindfunc(char *nm, int *siz)
{
	int p,f,len,n, swcode;
	char *ptr;
	if (!bc || !bc->mem) return -1; 

    /* read all chars from memory until a '\n' is encountered */
	f=-1;
	swcode=0;
	for (p=0; p<(int)bc->memsize; ) {
		ptr=&bc->mem[p];
		for (len=0; (*ptr!=0 && *ptr!='\n' && *ptr!='\r'); len++) ptr++;
		n=len;
		if (n>20) n=20;
		// printf("%5d - %4d %s %c : [%*.*s] ",p,len,nm,(f==-1)?' ':'x',n,n,&bc->mem[p]);
		if (*ptr!=0) {
			if (f==-1) {
				if (strncmp(&bc->mem[p],nm,len)==0) f=p+len+1;
			} else if (strncmp(&bc->mem[p],"1;",len)==0 || strncmp(&bc->mem[p],"PERLEOF",len)==0
				                                        || bc->mem[p]==':') {
				*siz=p-f;
				//	printf("found  %d  size=%d\n",f,*siz);
				return f;
			} else if (swcode==0 && bc->mem[p]==':') {	/* found next label so skip to next code pos*/
				f=p+len+1;
			} else {										/* found code */
				swcode=1;
			}
		}
		for (; (*ptr!=0 && (*ptr=='\n' || *ptr=='\r')); len++) ptr++;	// skip \r\n
		p+=len;
		// printf(" %d %d\n",len,p);
	}
	return -1;
}

int str2argv(char *fn,char *str) 
{
    int i;

	i=0;
	if (fn) argv[i++]=fn;
	for (; (*str && i<49); i++) {
		if (*str=='"') {
			str++;
			argv[i]=str;
			while (*str!=0 && *str!='"')  str++;
		} else {
			argv[i]=str;
			while (*str!=0 && *str!=' ')  str++;
		}
		if (*str!=0) *str++=0;
		while (*str==' ')  str++;
	}
	argv[i]=0;
	return i;
}


void doperlfunc(char *p)
{
  char funcnm[81], *script, *org, *errstr;
  char dirnm[201]; 
  int  i,len,pos;

  org=p;
  for (i=0; (*p!=' ' && *p!=0); i++)  funcnm[i]=*p++;
  funcnm[i]=0;
  errstr=0;
  TRACE("Perlcall [%s]%s\n",funcnm,p);
  getcwd(dirnm,200);
  SetEnvironmentVariable("plcfn",funcnm);
  SetEnvironmentVariable("plcdir",dirnm);
  if (funcnm[0]!=':') {
	  script=plc_fn(funcnm,0);
	  if (script) {
		  str2argv(funcnm,&org[0]);
		  errstr=PERLscript(script,argv);
		  if (errstr)  printf("%s\r\n",errstr);
		  free(script);
	  }
  } else {
	  pos=perlfindfunc(funcnm,&len);
	  if (bc && pos>=0) {
		  if (strncmp(&bc->mem[pos],"require ",8)==0) {
			  script=plc_fn(&bc->mem[pos],&len);
		  } else {
			  script=&bc->mem[pos];
		  }
		  if (script) {
			  char c;
			  str2argv(funcnm,org);
			  c=script[len];
			  script[len]=0;
			  // printf("script=[%s] len=%d\r\n",script,len);
			  errstr=PERLscript(script,argv);
			  if (errstr)  printf("%s\r\n",errstr);
			  script[len]=c;
			  free(script);
		  }
	  } else
		  printf("perlfunction not found... [%s]\r\n",funcnm);
  }
}

int  cmd_perlfile(char *fn,char *args)
{
  char *script,*errstr;
  TRACE("ExecPerl [%s] %s\n",fn,args);
  script=plc_fn(fn,0);
  if (script) {
	  char dirnm[201];
	  getcwd(dirnm,200);
	  SetEnvironmentVariable("plcfn",fn);
	  SetEnvironmentVariable("plcdir",dirnm);
	  str2argv(fn,args);
	  errstr=PERLscript(script,argv);
	  if (errstr) printf("%s\r\n",errstr);
	  TRACE("ExecPerl done\n");
	  free(script);
  }
  return (script)?0:1;
}


int	cmd_custom(char *c, char *p)
{
	if (strstr(c,"perlcall")!=0) {
		while (*p==' ') p++;
		doperlfunc(p);
		return 0;
	}
    return -1;
}

int cmd_sethook(int id, void *pFunc)
{
	switch (id) {
	   case CMDHOOK_LOADFILE : pCustomLoadFile=(tCMDHOOK_LoadFile)pFunc;  break;
	   case CMDHOOK_COMMAND  : pCustomCommand =(tCMDHOOK_Command )pFunc;  break;
	   case CMDHOOK_EXECUTE  : pCustomExecute =(tCMDHOOK_Execute )pFunc;  break;
	   default               : return 0;
	}
    return 1;
}

int DirExists(char *dirnm)
{
	DWORD attr;
	// test if it exists
	attr=GetFileAttributes(dirnm);
	if (attr==INVALID_FILE_ATTRIBUTES)  return 0;
	if (attr&FILE_ATTRIBUTE_DIRECTORY)  return 1;
	return 0;
}

void showsyntax(char *packnm)
{
    printf("# Perl/Cmd Script Executor                  Copyright 2006 - 2012\n");
    printf("# Version %4s                                     www.atbas.org\n",PLC_VERSION);
    printf("#\n");
	printf("# Syntax:  pCmd [/[C|K] command]\n");
    printf("#\n");
	printf("# Option:  /C command  Runs the specified command and terminates.\n");
	printf("#          /K command  Runs the specified command and remains.\n");
	printf("#          /I params   Runs commands from STDIN.\n");
    printf("#\n");
}

int main (int argc, char *argv[])
{
	int i, stat, typeplc;

	/* is a .pl or .plc file called */
	typeplc=0;
	for (i=1; (i<argc && typeplc==0); i++) {
		if (strstr(argv[i],".plc") || strstr(argv[i],".pl")) typeplc=i;
	}
	if (argc>1 && (argv[1][0]=='-' || (argv[1][0]=='/' && strchr("CKI",toupper(argv[1][1]))==0)) ) {
		showsyntax(0);
		exit(0);
	}

	/* Initialize perl interpreter */	
	PERLinit(argc,argv);
	//		PERLscript("$a=1;  print $a.\"\n\";",0,0);
	//		PERLscript("$a=$a+4;  print $a.\"\n\";",0,0);

	if (typeplc) {
      char *script, *errstr;
	  script=plc_fn(argv[typeplc],0);
	  if (script) {
	  	  char dirnm[201];
		  getcwd(dirnm,200);
		  SetEnvironmentVariable("plcfn",argv[typeplc]);
		  SetEnvironmentVariable("plcdir",dirnm);
		  errstr=PERLscript(script,&argv[typeplc]);
		  if (errstr) printf("%s\r\n",errstr);
		  free(script);
	  }
	} else {
		/* use custom functions in cmd interpreter */
		cmd_sethook(CMDHOOK_LOADFILE, (void *)plc_fn);
		cmd_sethook(CMDHOOK_COMMAND,  (void *)cmd_custom);
		cmd_sethook(CMDHOOK_EXECUTE,  (void *)cmd_perlfile);
		stat=cmd_main(argc,argv);
		PERLexit();
	}

	return stat;
}
