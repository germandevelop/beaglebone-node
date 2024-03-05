/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef GPIO_INT_TYPE_H_
#define GPIO_INT_TYPE_H_

#include <stddef.h>

typedef void (*gpio_int_isr_callback_t) (void *user_data);

typedef enum gpio_int_edge
{
    NONE = 0,
    RISING,
    FALLING,
    BOTH

} gpio_int_edge_t;

typedef struct gpio_int_isr
{
    size_t number;
    gpio_int_edge_t edge;
    gpio_int_isr_callback_t isr_callback;
    void *user_data;

} gpio_int_isr_t;

#endif // GPIO_INT_TYPE_H_
