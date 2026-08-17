#ifndef VERSION_STR_H
#define VERSION_STR_H

// Modify these as required.  Note that BUILD_NUM is auto-incremented
// with each build so that shouldn't need to be updated.
#define MAJOR_VER 0
#define MINOR_VER 9
#define PATCH_VER 9
#define BUILD_NUM 133

#define BETA_BUILD


// The rest of this shouldn't need to be updated in general and just
// coerces the above values into strings.
#define STRINGIFY(x) TO_STRING(x)
#define TO_STRING(x) #x

#define MAJOR_VER_STR STRINGIFY(MAJOR_VER)
#define MINOR_VER_STR STRINGIFY(MINOR_VER)
#define PATCH_VER_STR STRINGIFY(PATCH_VER)
#define BUILD_NUM_STR STRINGIFY(BUILD_NUM)


#define VERSION_STR MAJOR_VER_STR "." MINOR_VER_STR "." PATCH_VER_STR

#ifdef BETA_BUILD
#define VERSION_BUILD " (Beta build " BUILD_NUM_STR ")"
#else
#define VERSION_BUILD " (Release build)"
#endif

#define VERSION_DETAIL "BeBlue v" VERSION_STR VERSION_BUILD

#endif