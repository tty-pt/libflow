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

/**
 * Function pointer type for I/O
 *
 * @param port Index of the I/O port
 * @param data Pointer to the data to be read or written
 * @param len Length of the data to write
 */
typedef int flow_io_t(unsigned port,
		void *data, size_t len);

/**
 * Write information to the current node / flow output port
 *
 * @param port Index of the output port
 * @param data Pointer to the data to be written
 * @param len Length of the data to write
 */
flow_io_t flow_write;

/**
 * Read information from the current node / flow input port
 *
 * @param port Index of the input port
 * @param data Pointer to a copy of the data being read
 * @param len Length of the data being read
 */
flow_io_t flow_read;

/**
 * Function pointer type for linking transitions
 *
 * @param oinst ID of the node instance to transition from
 * @param iinst ID of the node instance to transition to
 */
typedef int flow_link_tran_t(unsigned oinst, unsigned iinst);

/**
 * Link a transition from a flow / node into another
 *
 * @param oinst ID of the node instance to transition from
 * @param iinst ID of the node instance to transition to
 */
flow_link_tran_t flow_link_tran;

/**
 * Function pointer type for linking normal ports
 *
 * @param oinst ID of the origin node instance
 * @param oport Index of the port in the origin node
 * @param iinst ID of the target node instance
 * @param iport Index of the port in that target node
 */
typedef int flow_link_t(unsigned oinst, 
		unsigned oport,
		unsigned iinst,
		unsigned iport);

/**
 * Link a port from a node into another
 *
 * @param oinst ID of the origin node instance
 * @param oport Index of the port in the origin node
 * @param iinst ID of the target node instance
 * @param iport Index of the port in that target node
 */
flow_link_t flow_link;

/**
 * Function pointer type for loading a node class
 *
 * @param name Name of the node class.
 * 	This is used to know what ".so" or ".py" file to load,
 * 	from the directory of flowd's execution.
 * @return The numeric id of the class
 */
typedef unsigned flow_node_t(char *name);

/**
 * Initialize (LOAD) a node / flow class.
 *
 * @param name Name of the node class.
 * 	This is used to know what ".so" or ".py" file to load,
 * 	from the directory of flowd's execution.
 * @return The numeric id of the class
 */
flow_node_t flow_node;

/**
 * Function pointer type for creating a node instance or transitioning into one
 *
 * @param id a numeric id
 * @return Numeric ID if needed
 */
typedef unsigned flow_inst_t(unsigned);

/**
 * Create an instance from a node class
 *
 * @param class_id The numeric ID of the class
 * 	to generate an instance for.
 * @return The numeric ID of the instance
 */
flow_inst_t flow_inst;

/**
 * Transition into a node instance (useful for flows)
 *
 * @param node_id The numeric ID of the instance to transition to
 */
flow_inst_t flow_tran;

/**
 * Function pointer type for a user-made function for running or loading a type of node
 */
typedef int flow_run_t();

/**
 * Implement flow_run yourself, which is the function that runs when a flow or node runs
 */
extern flow_run_t flow_run;

/**
 * Implement flow_init yourself, which is the function that runs when a flow or node loads.
 * This is where you load your nodes / sub-flows and link them between each-other.
 */
extern flow_run_t flow_init;

#endif
