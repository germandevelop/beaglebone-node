/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef MODULE_H_
#define MODULE_H_

typedef struct std_error std_error_t;

#ifdef __cplusplus
extern "C" {
#endif

int module_load (const char *module_path, std_error_t * const error);
int module_unload (const char *module_name, std_error_t * const error);
void module_unload_force (const char *module_name);

#ifdef __cplusplus
}
#endif

#endif // MODULE_H_
