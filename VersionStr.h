#ifndef VERSION_STR_H
#define VERSION_STR_H

#define BUILD_NUM 104

#define STRINGIFY(x) TO_STRING(x)
#define TO_STRING(x) #x

#define VERSION_STR "BeBlueCli v0.9.6 (build " STRINGIFY(BUILD_NUM) ")" 

#endif