/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "module.h"

#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "std_error/std_error.h"


int module_load (const char *module_path, std_error_t * const error)
{
    assert(module_path != NULL);

    int exit_code = STD_SUCCESS;

    const int file_descriptor = open(module_path, O_RDONLY);

    if (file_descriptor != (-1))
    {
        if (syscall(SYS_finit_module, file_descriptor, "", 0) != 0)
        {
            std_error_catch_errno(error, __FILE__, __LINE__);

            exit_code = STD_FAILURE;
        }
        close(file_descriptor);
    }
    else
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        exit_code = STD_FAILURE;
    }

    return exit_code;
}

int module_unload (const char *module_name, std_error_t * const error)
{
    assert(module_name != NULL);

    if (syscall(SYS_delete_module, module_name, O_NONBLOCK | O_EXCL) != 0)
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        return STD_FAILURE;
    }
    return STD_SUCCESS;
}

void module_unload_force (const char *module_name)
{
    assert(module_name != NULL);

    syscall(SYS_delete_module, module_name, O_NONBLOCK | O_EXCL | O_TRUNC);

    return;
}
