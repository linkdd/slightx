#ifndef __ARRAY_H
#define __ARRAY_H

/*!
 * \file array.h
 * \brief Interface for creating, inserting and deleting from ordered arrays.
 * \author David Delassus
 */

#include <types.h>

typedef void* type_t;

typedef int8_t (*lessthan_predicate_t)(type_t,type_t);
/*!
 * \struct ordered_array_t
 * \brief Describe an ordered array
 */
struct ordered_array_t
{
    type_t *array;
    uint32_t size;
    uint32_t max_size;
    lessthan_predicate_t less_than;
};

int8_t standard_lessthan_predicate (type_t a, type_t b);

struct ordered_array_t create_ordered_array (uint32_t max_size, lessthan_predicate_t less_than);
struct ordered_array_t place_ordered_array  (void *addr, uint32_t max_size, lessthan_predicate_t less_than);
void destroy_ordered_array (struct ordered_array_t *array);
void insert_ordered_array (type_t item, struct ordered_array_t *array);
type_t lookup_ordered_array (uint32_t i, struct ordered_array_t *array);
void remove_ordered_array (uint32_t i, struct ordered_array_t *array);

#endif /* __ARRAY_H */
