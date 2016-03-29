// tperl.cpp : Defines the entry point for the console application.
//

//#pragma comment(lib,"C:/Perl/lib/CORE/perl58s.lib")

#include "EXTERN.h"
#define PERL_IN_MINIPERLMAIN_C
#include "perl.h"
#include "XSUB.h"
#include "plib.h"
#pragma warning(disable: 4996)

static PerlInterpreter *my_perl;
static void xs_init (pTHX);

#pragma warning(push)
#pragma warning(disable : 4100)	// my_perl: unreferenced formal parameter

#include <EXTERN.h>
#include <perl.h>

//  statically link certain modules
//  perl -MExtUtils::Embed -e xsinit -- -o xsinit.c Socket
//  perl -MExtUtils::Embed -e ldopts -- -std Socket
//  need to build also static modules....!

EXTERN_C void xs_init (pTHX);

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);
//EXTERN_C void boot_Win32CORE  (pTHX_ CV* cv);
//EXTERN_C void boot_Socket (pTHX_ CV* cv);
//EXTERN_C void boot_File__Glob (pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX)
{
        char *file = __FILE__;
        dXSUB_SYS;

        /* DynaLoader is a special case */
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
//        newXS("Win32CORE::bootstrap", boot_Win32CORE, file);
//        newXS("Socket::bootstrap", boot_Socket, file);
//        newXS("File::Glob::bootstrap", boot_File__Glob, file);
}



#pragma warning(pop)

char *pargv[51];
int   pargc=7;

void setup_pargs(char *argv[])
{
	int n;
	pargv[0] = (argv)?argv[0]:"";		// filename
    pargv[1] = "-e";
	// unbuffered stdout and stderr, catch warnings into a var, can be retrieved when needed
	pargv[2] = "BEGIN{ select STDERR; $| = 1; select STDOUT; $| = 1; $SIG{__WARN__} = sub { $__w_warnings .= $_[0] } }";
	pargv[3] = "-S";
    pargv[4] = "-U";
    pargv[5] = "-f";
    pargv[6] = "-w";
    pargv[7] = "--";
    pargv[8] = 0;
	if (argv) {
		n=1;
    /* --- add program arguments */
        for (pargc = 8; (pargc < 50 && argv[n]!=0 && argv[n][0]!=0); pargc++)
            pargv[pargc] = argv[n++];
        pargv[pargc] = 0;
    } else
        pargc = 8;
}


void PERLcreate(int argc, char *argv[])
{
	// Much of this is straight out of perlembed. We are setting up an independent
	// Perl instance in its own thread. The 'BEGIN' line is to allow warning messages
	// that are normally sent to STDERR (and therefore lost in this environment) to be
	// stored in the $__w_warnings variable. This gives the user half a chance to debug
	// a script. The getWarnings() and clearWarnings() methods are used to get to this
	// variable
    my_perl = perl_alloc();
	if (my_perl) {
		PERL_SET_CONTEXT(my_perl);	// always do this in order to get correct thread
		PL_perl_destruct_level = 1;	// ensure proper and complete cleanup when closing Perl
	    perl_construct(my_perl);
		setup_pargs(0);
		perl_parse(my_perl, xs_init, pargc,pargv, (char **)NULL);	// use modules listed in xs_init()
		PL_exit_flags |= PERL_EXIT_DESTRUCT_END;	// run the END block when finished (post-5.7)
		perl_run(my_perl);
	}
}

void PERLinit(int argc, char *argv[])
{
	PERL_SYS_INIT3(&argc,&argv,NULL);
	PERLcreate(0,0);
}

void PERLdestroy()
{
	if (my_perl) {
		PERL_SET_CONTEXT(my_perl);	// ensure correct Perl thread
		perl_destruct((PerlInterpreter *)my_perl);
		perl_free((PerlInterpreter *)my_perl);
		my_perl = NULL;
	}
}


void PERLexit()
{
	PERLdestroy();
    PERL_SYS_TERM();
}

int PERLsetenv(char *name, char *val)
{
	// Create the scalar if it doesn't exist, turn off spurious warnings
	SV *v = get_sv(name, TRUE|GV_ADDMULTI);
	if(!v) return 0;
	sv_setpv(v, val?val:"");
	return 1;
}

int PERLsetEnv(char *hname)
{
	int i;
	char *ptr, str[512], *lpEnv, *lpOutput;
    SV *svval;
    // Create the hash if it doesn't exist, turn off spurious warnings
    HV *v = get_hv(hname, TRUE|GV_ADDMULTI);
    if(!v) return 0;
    if (strcmp(hname,"ENV")!=0)  hv_clear(v);     // clear out all previous values (if any) except for ENV

	i=0;
	lpEnv = (LPTSTR)GetEnvironmentStrings();
	if (lpEnv)
	{
		lpOutput = lpEnv;
		while (*lpOutput)
		{
			if (*lpOutput != '=') {
				strcpy_s(str,511,lpOutput);
				ptr=strchr(str,'=');
				if (ptr) {
					*ptr='\0';
			        svval = newSVpv(&ptr[1],0);     // make a string SV
			        if(svval) hv_store(v,str,strlen(str), svval, 0);
					i++;
				}
			}
			lpOutput += strlen(lpOutput) + 1;
		}
		FreeEnvironmentStrings (lpEnv);
	}
    return i;
}

int PERLsetArg(char *anm,char *argv[])
{
	int loop;
	// Create the array if it doesn't exist, turn off spurious warnings
	AV *v = get_av(anm, TRUE|GV_ADDMULTI);
	if(!v) return 0;
	av_clear(v);	// clear out all previous values (if any)

	for(loop = 0; argv[loop]!=0; loop++)
	{
		SV *val = newSVpv(argv[loop],strlen(argv[loop]));
		if(!val) return 0;
//		printf("PARAM %2d = [%s]\n",loop,argv[loop]);
		av_push(v, val);
	}
	return loop;
}

int PERLretvals(char *hname)
{
	HE *hashEntry;	// pointer to current hashentry
	long keylen;
	char keybuf[256];	// bigger-hammer-is-better school of software engineering
    char *keyname,*val;
	SV *sv;

	// Get the hash if it exists
	HV *v = get_hv(hname, FALSE);
	if(!v) {
//		printf ("NOT found %s\n",hname);
		return 0;
	}
	// Now we have to get a list of keys
	hv_iterinit(v);	// get beginning iterator
	while((hashEntry = hv_iternext(v)) != NULL)
	{
		// Get the key
		keyname = hv_iterkey(hashEntry, &keylen);
		if(!keyname) return 0;

		strncpy(keybuf,keyname,keylen);
		keybuf[keylen] = 0;
//		printf("found %s\n",keybuf);
		// Now get the value -- its OK to have an empty value in a hash
		sv = hv_iterval(v, hashEntry);
		if (!sv)
		{
			SetEnvironmentVariable(keyname,NULL);
		}
		else
		{
			val = SvPV_nolen(sv);
			SetEnvironmentVariable(keybuf,val);
//			printf("set %s=%s\n",keybuf,val);
		}
	}
	return 1;
}


void PERLreset(char *av[],char *script)
{

	if (my_perl) {
		PERL_SET_CONTEXT(my_perl);	// always do this in order to get correct thread
		PL_perl_destruct_level = 1;	// ensure proper and complete cleanup when closing Perl
		PL_exit_flags |= PERL_EXIT_DESTRUCT_END;	// run the END block when finished (post-5.7)
		perl_construct(my_perl);
		setup_pargs(av);
		perl_parse(my_perl, xs_init, pargc, pargv, (char **)NULL);	// use modules listed in xs_init()
	}
}

char *PERLscript(char *script, char *argv[])
{
	STRLEN n_a;
	static char   *errmsg;
	if (my_perl) {
	    // set console to standard mode
  	    SetConsoleMode (GetStdHandle(STD_INPUT_HANDLE),
		                ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT );
		PERLreset(argv,0);
//		PERLsetEnv("ENV");
//		perl_run(my_perl);
		eval_pv(script, FALSE);	// eval(p) and don't abort if 'p' throws an exception
		if(SvTRUE(ERRSV)) {			    /* check $@ */
			errmsg=SvPV(ERRSV,n_a);
			return errmsg;
		}
//		PERLretvals("RENV");
	}
	return 0;
}

// set PERL5LIB=C:\Perl\lib;C:\Perl\site\lib 
