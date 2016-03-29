#include <precomp.h>

INT WINAPI
DllMain(
   IN PVOID hInstanceDll,
   IN ULONG dwReason,
   IN PVOID reserved)
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         cmd_main(0, NULL, hInstanceDll);
         break;

      case DLL_THREAD_ATTACH:
         break;

      case DLL_THREAD_DETACH:
         break;

      case DLL_PROCESS_DETACH:
         break;
   }

   return TRUE;
}

int _tmain (int argc, const TCHAR *argv[])
{
    return cmd_main(argc, argv, NULL);
}
