/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "gpio_int.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "std_error/std_error.h"


#define FILE_READING_ERROR_TEXT "File reading error"
#define FILE_WRITING_ERROR_TEXT "File writing error"
#define THREAD_ERROR_TEXT       "Thread error"
#define MALLOC_ERROR_TEXT       "Malloc error"
#define SIGNAL_ERROR_TEXT       "Signal error"

#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))


static volatile sig_atomic_t is_signal_registered = (sig_atomic_t)(false);


static void gpio_int_usr1_signal_handler (int signal_number);

static int gpio_int_start (void *thrd_arg);

static int gpio_int_set_export (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error);
static int gpio_int_set_direction (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error);
static int gpio_int_set_edge (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error);

int gpio_int_init (gpio_int_t * const self,
                    gpio_int_config_t const * const init_config,
                    std_error_t * const error)
{
    assert(self != NULL);
    assert(init_config != NULL);
    assert(init_config->gpio_count > 0U);

    if (is_signal_registered != (sig_atomic_t)(true))
    {
        signal(SIGUSR1, gpio_int_usr1_signal_handler);

        is_signal_registered = (sig_atomic_t)(true);
    }

    self->gpio_array = (gpio_int_isr_t*)malloc(init_config->gpio_count * sizeof(gpio_int_isr_t));

    if (self->gpio_array == NULL)
    {
        std_error_catch_custom(error, STD_FAILURE, MALLOC_ERROR_TEXT, __FILE__, __LINE__);

        return STD_FAILURE;
    }

    self->gpio_size     = 0U;
    self->gpio_capacity = init_config->gpio_count;

    return STD_SUCCESS;
}

void gpio_int_deinit (gpio_int_t * const self)
{
    assert(self != NULL);

    self->gpio_size     = 0U;
    self->gpio_capacity = 0U;

    free(self->gpio_array);
    self->gpio_array = NULL;

    return;
}

int gpio_int_register_isr (gpio_int_t * const self,
                            gpio_int_isr_t const * const gpio_isr,
                            std_error_t * const error)
{
    assert(self                     != NULL);
    assert(gpio_isr                 != NULL);
    assert(gpio_isr->isr_callback   != NULL);
    assert(self->gpio_size          != self->gpio_capacity);

    int exit_code = gpio_int_set_export(self, gpio_isr, error);

    if (exit_code != STD_FAILURE)
    {
        exit_code = gpio_int_set_direction(self, gpio_isr, error);

        if (exit_code != STD_FAILURE)
        {
            exit_code = gpio_int_set_edge(self, gpio_isr, error);

            if (exit_code != STD_FAILURE)
            {
                self->gpio_array[self->gpio_size] = *gpio_isr;
                ++self->gpio_size;
            }
        }
    }
    return exit_code;
}

int gpio_int_start_thread (gpio_int_t *self, std_error_t * const error)
{
    assert(self != NULL);

    self->is_stop = false;

    const int error_code = thrd_create(&self->thread, (thrd_start_t)gpio_int_start, (void*)self);

    if (error_code != thrd_success)
    {
        std_error_catch_custom(error, error_code, THREAD_ERROR_TEXT, __FILE__, __LINE__);

        return STD_FAILURE;
    }
    return STD_SUCCESS;
}

int gpio_int_stop_thread (gpio_int_t * const self, std_error_t * const error)
{
    assert(self != NULL);
    
    self->is_stop = true;

    int exit_code = raise(SIGUSR1);

    if (exit_code != 0)
    {
        std_error_catch_custom(error, exit_code, SIGNAL_ERROR_TEXT, __FILE__, __LINE__);

        return STD_FAILURE;   
    }

    int thread_exit_code;
    exit_code = thrd_join(self->thread, &thread_exit_code);

    if (exit_code != thrd_success)
    {
        std_error_catch_custom(error, exit_code, THREAD_ERROR_TEXT, __FILE__, __LINE__);

        return STD_FAILURE;   
    }

    return STD_SUCCESS;
}


int gpio_int_start (void *thrd_arg)
{
    gpio_int_t *self = (gpio_int_t*)thrd_arg;

    struct pollfd value_pollfd_array[self->gpio_size];

    for (size_t i = 0U; i < self->gpio_size; ++i)
    {
        char value_path[64];
        sprintf(value_path, "/sys/class/gpio/gpio%u/value", self->gpio_array[i].number);

        value_pollfd_array[i].fd        = open(value_path, O_RDONLY | O_NONBLOCK);
        value_pollfd_array[i].events    = POLLPRI | POLLERR ;
        value_pollfd_array[i].revents   = 0;
    }

    while (true)
    {
        const int exit_code = poll(value_pollfd_array, self->gpio_size, -1);

        if (exit_code > 0)
        {
            for (size_t i = 0U; i < self->gpio_size; ++i)
            {
                if ((value_pollfd_array[i].revents & POLLPRI) > 0)
                {
                    lseek(value_pollfd_array[i].fd, 0, SEEK_SET);

                    uint8_t value;
                    ssize_t bytes = read(value_pollfd_array[i].fd, (void*)&value, 1U);
                    UNUSED(bytes);

                    self->gpio_array[i].isr_callback(self->gpio_array[i].user_data);
                }
            }
        }

        if (self->is_stop == true)
        {
            break;
        }
    }
    return STD_SUCCESS;
}


void gpio_int_usr1_signal_handler (int signal_number)
{
    UNUSED(signal_number);
    UNUSED(is_signal_registered);

    // Do nothing - just to interrupt poll()

    return;
}


int gpio_int_set_export (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error)
{
    UNUSED(self);

    int exit_code = STD_SUCCESS;

    char isr_number[8];
    const size_t isr_number_length = (size_t)sprintf(isr_number, "%u", gpio_isr->number);

    char value_path[64];
    sprintf(value_path, "/sys/class/gpio/gpio%u/value", gpio_isr->number);

    // Try to enable gpio
    if (access(value_path, F_OK) != 0)
    {
        const int export_fd = open("/sys/class/gpio/export", O_WRONLY);

        if (export_fd != (-1))
        {
            if (write(export_fd, (const void*)isr_number, isr_number_length) == (-1))
            {
                std_error_catch_errno(error, __FILE__, __LINE__);

                exit_code = STD_FAILURE;
            }
            close(export_fd);
        }
        else
        {
            std_error_catch_errno(error, __FILE__, __LINE__);

            exit_code = STD_FAILURE;
        }
    }
    return exit_code;
}

int gpio_int_set_direction (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error)
{
    UNUSED(self);

    int exit_code = STD_SUCCESS;

    char direction_path[64];
    sprintf(direction_path, "/sys/class/gpio/gpio%u/direction", gpio_isr->number);

    // Try to enable gpio
    FILE *direction_file = fopen(direction_path, "r+");

    if (direction_file != NULL)
    {
        fputs("in", direction_file);

        if (ferror(direction_file) != 0)
        {
            std_error_catch_custom(error, ferror(direction_file), FILE_WRITING_ERROR_TEXT, __FILE__, __LINE__);

            exit_code = STD_FAILURE;
        }
        fclose(direction_file);
    }
    else
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        exit_code = STD_FAILURE;
    }
    return exit_code;
}

int gpio_int_set_edge (gpio_int_t const * const self, gpio_int_isr_t const * const gpio_isr, std_error_t * const error)
{
    UNUSED(self);

    int exit_code = STD_SUCCESS;

    char edge_path[64];
    sprintf(edge_path, "/sys/class/gpio/gpio%u/edge", gpio_isr->number);

    // Try to enable gpio
    FILE *edge_file = fopen(edge_path, "r+");

    if (edge_file != NULL)
    {
        const char edge_text[][8] =
        {
            [NONE]      = "none",
            [RISING]    = "rising",
            [FALLING]   = "falling",
            [BOTH]      = "both"
        };

        fputs(edge_text[gpio_isr->edge], edge_file);

        if (ferror(edge_file) != 0)
        {
            std_error_catch_custom(error, ferror(edge_file), FILE_WRITING_ERROR_TEXT, __FILE__, __LINE__);

            exit_code = STD_FAILURE;
        }
        fclose(edge_file);
    }
    else
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        exit_code = STD_FAILURE;
    }
    return exit_code;
}
