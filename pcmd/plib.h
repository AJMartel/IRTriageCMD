#pragma once

// disable depreciate functions, /GS optimisation and pack warnings
#pragma warning(disable: 4996 4748 4103)

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

void PERLinit(int argc, char *argv[]);
void PERLexit(void);
char *PERLscript(char * script, char **argv);

#ifdef __cplusplus
}
#endif
