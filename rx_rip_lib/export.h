#ifdef WIN32
	#define EXPORT EXTERN_C _declspec(dllexport) 
#else 
	#define EXPORT
#endif
