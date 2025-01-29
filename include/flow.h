/**
 * @file flow.h
 * @brief Header file for the Flow library.
 *
 * This file contains all the necessary definitions, types, and functions
 * for managing flows and nodes.
 */

#ifndef FLOW_H
#define FLOW_H

#include <stddef.h>

typedef int flow_io_t(unsigned port, void *data, size_t len);

/**
 * @brief Write information to the current node / flow output port
 *
 * @param port Index of the output port
 * @param data Pointer to the data to be written
 * @param len Length of the data to write
 */
int flow_write(unsigned port, void *data, size_t len);

/**
 * @brief Read information from the current node / flow input port
 *
 * @param port Index of the input port
 * @param data Pointer to a copy of the data being read
 * @param len Length of the data being read
 */
int flow_read(unsigned port, void *data, size_t len);

typedef int flow_link_tran_t(unsigned oinst, unsigned iinst);

/**
 * @brief Link a transition from a flow / node into another
 *
 * @param oinst ID of the node instance to transition from
 * @param iinst ID of the node instance to transition to
 */
int flow_link_tran(unsigned oinst, unsigned iinst);

typedef int flow_link_t(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport);

/**
 * @brief Link a port from a node into another
 *
 * @param oinst ID of the origin node instance
 * @param oport Index of the port in the origin node
 * @param iinst ID of the target node instance
 * @param iport Index of the port in that target node
 */
int flow_link(unsigned oinst, unsigned oport,
		unsigned iinst, unsigned iport);

typedef unsigned flow_node_t(char *name);

/**
 * @brief Initialize (LOAD) a node / flow class.
 *
 * @param name Name of the node class.
 * 	This is used to know what ".so" or ".py" file to load,
 * 	from the directory of flowd's execution.
 * @return The numeric id of the class
 */
unsigned flow_node(char *name);

typedef unsigned flow_inst_t(unsigned);

/**
 * @brief Create an instance from a node class
 *
 * @param class_id The numeric ID of the class
 * 	to generate an instance for.
 * @return The numeric ID of the instance
 */
unsigned flow_inst(unsigned class_id);

/**
 * @brief Transition into a node instance (useful for flows)
 *
 * @param node_id The numeric ID of the instance to transition to
 */
unsigned flow_tran(unsigned class_id);

typedef int flow_run_t();

/**
 * Implement flow_run yourself, which is the function that runs when a flow or node runs
 */
extern int flow_run();

/**
 * Implement flow_init yourself, which is the function that runs when a flow or node loads.
 * This is where you load your nodes / sub-flows and link them between each-other.
 */
extern int flow_init();

#endif
