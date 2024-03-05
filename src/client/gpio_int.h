/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef GPIO_INT_H_
#define GPIO_INT_H_

#include "gpio_int.type.h"

typedef struct gpio_int gpio_int_t;
typedef struct std_error std_error_t;

typedef struct gpio_int_config
{
    size_t gpio_count;

} gpio_int_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int gpio_int_init (gpio_int_t * const self,
                    gpio_int_config_t const * const init_config,
                    std_error_t * const error);
void gpio_int_deinit (gpio_int_t * const self);

int gpio_int_register_isr (gpio_int_t * const self,
                            gpio_int_isr_t const * const gpio_isr,
                            std_error_t * const error);

int gpio_int_start_thread (gpio_int_t *self, std_error_t * const error);
int gpio_int_stop_thread (gpio_int_t * const self, std_error_t * const error);

#ifdef __cplusplus
}
#endif



// Private
#include <threads.h>

#ifdef __cplusplus
#include <atomic>
using namespace std; // Ugly, like a monster
#else
#include <stdatomic.h>
#endif

typedef struct gpio_int
{
    gpio_int_isr_t *gpio_array;
    size_t gpio_capacity;
    size_t gpio_size;

    thrd_t thread;
    atomic_bool is_stop;

} gpio_int_t;

#endif // GPIO_INT_H_
