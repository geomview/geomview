/*		@(#)ucd_defs.h	7.7 Stardent 92/03/20	*/
/*
			Copyright (c) 1989 by
			Stardent Computer Inc.
			All Rights Reserved
	
	This software comprises unpublished confidential information of
	Stardent Computer Inc. and may not be used, copied or made
	available to anyone, except in accordance with the license
	under which it is furnished.
	
	This file is under sccs control at AVS in:
	@europa/sccs1.p/avs/include/s.ucd_defs.h
	
*/
/*#ifdef SX*/
/* I don't like to see port-specific differences like this. */
/* KCI thought it was necessary for SX-3. */
/* See if it causes trouble on others. */
#   include <avs/port.h>
/*#endif SX*/
 

/*---------------------------
   Cell Type Definitions      	
 ---------------------------*/

#define UCD_NTYPES		8
#define UCD_NUM_CELL_TYPES	8

#define UCD_POINT		0
#define UCD_LINE		1
#define	UCD_TRIANGLE		2
#define UCD_QUADRILATERAL	3
#define UCD_TETRAHEDRON		4
#define UCD_PYRAMID		5
#define UCD_PRISM		6
#define UCD_HEXAHEDRON		7

#define UCD_MAX_NODE_PER_CELL    20

/*---------------------------
   Miscellaneous Defines      	
 ---------------------------*/

#define UCD_MATERIAL_IDS  0x1
#define UCD_NODE_NAMES    0x2
#define UCD_CELL_NAMES    0x4
#define UCD_CELL_TYPES    0x8
#define UCD_MID_EDGES     0x10
#define UCD_CHAR          0x20
#define UCD_INT           0x40

#define UCD_LABEL_LEN         1024
#define UCD_MAX_COMPONENTS    50
#define UCD_MAX_VECLEN        9


/*---------------------------
   Cell Type Variables      	
 ---------------------------*/

extern	int	UCD_num_nodes[];	/* number of nodes in a cell */
extern	int	UCD_num_mid_nodes[];	/* number of nodes in a cell */
					/* with mid-edge nodes	     */
extern	int	UCD_num_faces[];	/* number of faces in a cell */

 
 

/*-----------------------------------------
   Unstructured Cell Data Type Definitions
 -----------------------------------------*/


typedef union {				/* name union                        */
 	char		*c;		/* character string for label names  */
 	int		i;		/* integer for numerical names       */
}  UCD_name;  



/*---------------------------------------
   NOTE:  in the util_flag field, the two
	  rightmost bits are reserved for
	  internal usage.
 -----------------------------------------*/


typedef struct UCD_structure_ {	
 
 	/*--------- Structure Header Information ---------*/
 	char		*name;		 /* structure name                    */
 	int		name_flag;	 /* are node/cell names chars or ints */
 	int		ncells;		 /* number of cells                   */
 	int		nnodes;		 /* number of nodes                   */
	float		min_extent[3];	 /* structure extent		      */
	float		max_extent[3];	 /* structure extent		      */
 	int		data_veclen;	 /* length of data vector for struct  */
	float		*data;		 /* data for the structure	      */
	char		*data_labels;	 /* labels for data components        */
	char		*data_units;	 /* labels for data units             */
	int		util_flag;  	 /* utility flag (see above for any   */
					 /* restrictions) 		      */
 
 
 	/*--------- Cell Information ---------*/
	UCD_name	*cell_name;	 /* cell names		              */
 	char		**element_type;	 /* cell element types                */
 	int		*material_type;	 /* user defined material types       */
 	int		*cell_type;	 /* cell types (see above defines)    */
 	int		cell_veclen;	 /* length of data vector             */
 	float		*cell_data;	 /* data for cell-based datasets      */
	float		*min_cell_data;  /* min val for cell data components  */
	float		*max_cell_data;  /* max val for cell data components  */
	char		*cell_labels;	 /* labels for cell data components   */
	char		*cell_units;	 /* labels for cell data units        */
	int		*cell_components;/* array of cell component mix       */
	int		*cell_active;    /* array of active cell components   */
 	int		*mid_edge_flags; /* cell edges with mid edge nodes    */
 	int		node_conn_size;  /* size of the node connectivity list*/
 	int		*node_list;	 /* node list of connectivity         */
 	int		*node_list_ptr;  /* location of a cell's node list    */
 	int		ucd_last_cell;  
 
  
 	/*--------- Node Information ---------*/
	UCD_name	*node_name;	 /* node names              	      */
 	float		*x, *y, *z;	 /* position of the nodes             */
 	int		node_veclen;	 /* length of data vector             */
 	float		*node_data;	 /* data vector for the nodes         */
	float		*min_node_data;  /* min val for node data components  */
	float		*max_node_data;  /* max val for node data components  */
	char		*node_labels;	 /* labels for node data components   */
	char		*node_units; 	 /* labels for node data units        */
	int		*node_components;/* array of node component mix       */
	int		*node_active;    /* array of active node components   */
 	int		cell_conn_size;  /* size of the cell connectivity list*/
 	int		*cell_list;	 /* cell list of connectivity         */
 	int		*cell_list_ptr;  /* location of a node's cell list    */
 	int		ucd_last_node;  


	/*--------- Allocation Information ----------*/
	enum {
		UCD_ONE_BLOCK,
		UCD_RW_SHM,
		UCD_RO_SHM
	} 		alloc_case;	/* storage allocation strategy	      */
	int		shm_key;	/* shared memory key		      */
	int		shm_id;		/* shared memory id		      */
	char		*shm_base;	/* shared memory base		      */
	int             mesh_id;        /* unique id of the mesh instance     */

} UCD_structure;



/*----------------------------------------------------------------------------
  ----------------------------------------------------------------------------

    Function Headers for the routines handling Unstructured Cell Data (UCD)

  ----------------------------------------------------------------------------
  ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
char  *UCDstructure_alloc (name, data_veclen, name_flag, 
			   ncells, cell_count, cell_veclen,
		           nnodes, node_count, node_veclen, util_flag)
char		*name;
int		data_veclen;
int		name_flag;
int		ncells;
int		cell_count;
int		cell_veclen;
int		nnodes;
int		node_count;
int		node_veclen;
int		util_flag;

This function creates a new top level structure and returns a pointer
to that structure.

Inputs:		name		- structure name
		data_veclen	- length of structure data vector 
		name_flag	- are node/cell names chars or ints
					char = UCD_CHAR,  int = UCD_INT
		ncells		- number of cells in the structure
		cell_count	- expected number of nodes per cell
		cell_veclen	- length of cell data vector
		nnodes		- number of nodes in the structure 
		node_count	- expected number of cells per node
		node_veclen	- length of node data vector
		util_flag	- utility flag for general usage
Outputs: 			- none
Returns:			- pointer to the new structure
				  pointer = NULL if error

Description:

	The UCD_structure contains pointers to several blocks of memory.
	They are arranged in two groups as shown below:


		+-----------------------+  \
		|			|   \
		|   UCD_structure	|    |
		|			|    |
		|			|    |
		| name, data, 		|    |
	   |====| data_labels, 		|    |
	   |====| element_type,		|    |
	   ||	| etc., etc., etc.	|    |
	   ||	|			|    |
	|=======| cell_data, 		|    |
	|=======| x, y, z,		|     }  msize
	|| ||	| connectivity lists	|    |
	|| ||	| node_data		|    |
	|| ||	|			|    |
	|| ||	+-----------------------+    |
	|| |==>>|			|    |
	|| ||	|			|    |
	|| ||	|			|    |
	|| |==>>|			|    |
	||	| (Some of these are	|    |
	||	| arrays of pointers	|    |
	||	| to additional pieces	|    |
	||	| of malloc'ed memory.)	|    |
	||	|			|   /
	||	+-----------------------+  /
	||
	||	+-----------------------+  \
	|=====>>|			|   \
	||	|			|    |
	||	| (All of these		|     }  ssize
	|=====>>| are floats or ints.)	|    |
		|			|    |
		|			|   /
		+-----------------------+  /

	
	compute_sizes calculates the required memory block sizes msize 
	and ssize.  The two blocks shown above can either be allocated
	in one big chunk or be in separate regions of memory.

	The UCDstructure and most of the arrays it points to are allocated
	in one big block of msize bytes.  This is allocated by malloc.
	carve_up_the_memory fills in all the pointers to appropriate
	positions in this block of memory.

	The cell_data, x, y, z, connectivity lists and node_data arrays 
	tend to require very large amounts of memory so it is beneficial 
	to put them in shared memory when possible.  When not appropriate, 
	both blocks shown above can be obtained with a single malloc.

	The various cases are encoded in fields of UCDstructure:

	case 1 - Everything in one big malloc'ed block.

		This was the original implementation.  Will still be used
		on sytems without shared memory or if the shared memory
		allocation fails for any reason.

		alloc_case = UCD_ONE_BLOCK
		shm_key not used
		shm_id not used
		shm_base not used

	case 2 - Second block is in Read/Write shared memory.

		This is the normal case for the producer of UCD.
		
		alloc_case = UCD_RW_SHM
		shm_key = shared memory key 
		shm_id = shared memory identifier
		shm_base = base address of the shared memory

	case 3 - Second block is in Read-Only shared memory.

		This is the normal case for the consumer of UCD.

		The same UCD could be attached to more than one input
		port so we don't want a consumer making any changes to it.
		Therefore, the shared memory is made read-only.

		alloc_case = UCD_RO_SHM
		shm_key = shared memory key  (this is passed thru the socket)
		shm_id = shared memory identifier  (each process must 
				obtain this from shmget because each process
				might have a different id for the same
				shared memory segment)
		shm_base = base address of the shared memory  (each process
				must obtain this from shmat because each
				process might have the shared memory
				segment at a different address)

	case 4 - Second block is in own malloc'ed block.

		Sometimes the consumer of a UCD will want to modify it
		in place - to normalize it or something.

		The module writer can prevent any of the UCD from being
		in Read-Only storage by specifying the MODIFY_IN option
		in AVScreate_input_port.

		I'm sure not what to do about this yet.

		If it is not convenient to do something earlier, anything
		in read-only shared memory will get copied into a malloc'ed
		block and all the pointers will be fixed up.

		TO BE CONTINUED .....

-----------------------------------------------------------------------------*/
extern char  *UCDstructure_alloc ();



/*----------------------------------------------------------------------------
int UCDstructure_free (structure)
UCD_structure *structure;

This function frees the storage used by structure.

Inputs:	structure		- structure to free
Outputs:			- none
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_free ();
 


/*----------------------------------------------------------------------------
int UCDstructure_get_header (structure, name, data_veclen, name_flag, ncells, 
			     cell_veclen, nnodes, node_veclen, util_flag)
UCD_structure	*structure;
char		*name;
int		*data_veclen;
int		*name_flag;
int		*ncells;
int		*cell_veclen;
int		*nnodes;
int             *node_veclen;
int		*util_flag;

This function finds out all the header information about a UCD_structure and
returns those values.

Inputs:		structure	- structure to find information
Outputs:	name		- structure name
		data_veclen	- length of structure data vector 
		name_flag	- are node/cell names chars or ints
					char = UCD_CHAR,  int = UCD_INT
		ncells		- number of cells in the structure
		cell_veclen	- length of cell data vector 
 		nnodes		- number of nodes in the structure
		node_veclen	- length of node data vector 
		util_flag	- utility flag
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_header ();



/*----------------------------------------------------------------------------
int UCDstructure_set_header_flag (structure, util_flag)
UCD_structure	*structure;
int		util_flag;     

This function sets the header flag bits.

Inputs:		structure		- structure to find information
         	util_flag		- utility flag
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_header_flag ();



/*----------------------------------------------------------------------------
int UCDstructure_get_data (structure, data)
UCD_structure	*structure;
float		**data;     

This function returns a pointer to the array containing the data vector
for the structure.  
 
Inputs:		structure	- structure to find information
Outputs:	data		- pointer to the structure data vector
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_data ();



/*----------------------------------------------------------------------------
int UCDstructure_set_data (structure, data)
UCD_structure	*structure;
float		*data;     

This function copies the data from the array pointed to by "data"
into the structure's data array.  There should be data_veclen
data elements in this array.

Inputs:		structure	- structure to find information
        	data		- pointer to the data vector
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_data ();



/*----------------------------------------------------------------------------
int UCDstructure_get_extent (structure, min_extent, max_extent)
UCD_structure	*structure;
float		*min_extent;
float		*max_extent;

This routine allows the module writer to obtain the extent of the
structure. 


Inputs:		structure		- structure to find information
Outputs: 	min_extent 		- coordinate extent of structure
         	max_extent 		- coordinate extent of structure
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_extent ();



/*----------------------------------------------------------------------------
int UCDstructure_set_extent (structure, min_extent, max_extent)
UCD_structure	*structure;
float		*min_extent;
float		*max_extent;

This routine allows the module writer to set the extent of the
structure. 

Inputs:		structure		- structure to find information
         	min_extent 		- coordinate extent of structure
         	max_extent 		- coordinate extent of structure
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_extent ();



/*----------------------------------------------------------------------------
int UCDstructure_get_data_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the labels for each
component in the structure. These labels are for cases when there is 
structure based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_data_labels ();



/*--------------------------------------------------------------------
int  UCDstructure_get_data_label (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
component in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_data_label ();



/*----------------------------------------------------------------------------
int UCDstructure_set_data_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the labels for each
component in the structure. These labels are for cases when there is 
structure based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_data_labels ();



/*----------------------------------------------------------------------------
int UCDstructure_get_cell_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the labels for each
component in the structure. These labels are for cases when there is 
cell based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_cell_labels ();



/*--------------------------------------------------------------------
int  UCDstructure_get_cell_label (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
component in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_cell_label ();



/*----------------------------------------------------------------------------
int UCDstructure_set_cell_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the labels for each
component in the structure. These labels are for cases when there is 
cell based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_cell_labels ();



/*----------------------------------------------------------------------------
int UCDstructure_get_node_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the labels for each
component in the structure. These labels are for cases when there is 
node based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_labels ();



/*--------------------------------------------------------------------
int  UCDstructure_get_node_label (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
component in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_node_label ();



/*----------------------------------------------------------------------------
int UCDstructure_set_node_labels (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the labels for each
component in the structure. These labels are for cases when there is 
node based data.  

For instance, in the case of a CFD dataset,
the module writer might want to label components of the field as
temperature, density, mach number, etc.  In turn, these labels would
appear on the dials so that the user would have a better understanding
of which component each dial is attached to.

   Example:   labels    = "temp;density;mach number"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_labels ();


/*----------------------------------------------------------------------------
int UCDstructure_get_data_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the unit labels for each
component in the structure. These labels are for cases when there is 
structure based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_data_units ();



/*--------------------------------------------------------------------
int  UCDstructure_get_data_unit (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
unit in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_data_unit ();



/*----------------------------------------------------------------------------
int UCDstructure_set_data_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the unit labels for each
component in the structure. These labels are for cases when there is 
structure based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_data_units ();



/*----------------------------------------------------------------------------
int UCDstructure_get_cell_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the unit labels for each
component in the structure. These labels are for cases when there is 
cell based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_cell_units ();



/*--------------------------------------------------------------------
int  UCDstructure_get_cell_unit (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
unit in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_cell_unit ();



/*----------------------------------------------------------------------------
int UCDstructure_set_cell_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the unit labels for each
component in the structure. These labels are for cases when there is 
cell based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_cell_units ();



/*----------------------------------------------------------------------------
int UCDstructure_get_node_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to get the unit labels for each
component in the structure. These labels are for cases when there is 
node based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
Outputs: 	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_units ();



/*--------------------------------------------------------------------
int  UCDstructure_get_node_unit (structure, number, label)
UCD_structure	*structure;
int 	        number; 
char            *label;   

This routine allows the module writer to query the label for an individual 
unit in the structure.  

Input:		structure	-- structure to get labels in
		number		-- individual component number
Outputs:	labels		-- string with labels included
Returns:			-- 1 if success;  0 if failure
----------------------------------------------------------------------*/
extern int  UCDstructure_get_node_unit ();



/*----------------------------------------------------------------------------
int UCDstructure_set_node_units (structure, labels, delimiter)
UCD_structure	*structure;
char 		*labels;
char 		*delimiter;

This routine allows the module writer to set the unit labels for each
component in the structure. These labels are for cases when there is 
node based data.  

For instance, in the case of a CFD dataset, the module writer might want 
to label components of the field as degrees, meters, etc.  

   Example:   labels    = "degrees;meters"
              delimiter = ";"

Inputs:		structure		- structure to find information
         	labels     		- string with labels included   
         	delimiter  		- delimiter between each label  
Returns:				- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_units ();



/*---------------------------------------------------------------------------
int UCDstructure_invalid_cell_minmax (structure)
UCD_structure        *structure;

This routine allows the module writer to set the min/max range of the
structure cell data to be invalid.  This function should be used after 
the structure data has been changed by the module and the module does 
not want to spend the time recomputing the cell minmax.                   

Input:          structure       -- structure to set cell min/max invalid
Outputs:                        -- none
Returns:                        -- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_invalid_cell_minmax ();



/*---------------------------------------------------------------------------
int  UCDstructure_get_cell_minmax (structure, min, max)
UCD_structure   *structure;
float           *min;
float           *max;

This routine allows the module writer to obtain the range of the
structure cell data.  It should be noted that min and max are arrays of
dimension structure->cell_veclen.

Input:          structure       -- structure to get min/max in
Outputs:        min             -- value of minimum data point
		max             -- value of maximum data point
Returns:                        -- 1 = valid data;  0 = invalid data
-----------------------------------------------------------------------------*/
extern int  UCDstructure_get_cell_minmax ();



/*---------------------------------------------------------------------------
int  UCDstructure_set_cell_minmax (structure, min, max)
UCD_structure   *structure;
float           *min;
float           *max;

This routine allows the module writer to set the range of the
structure cell data.  It should be noted that min and max are arrays of
dimension structure->cell_veclen.

Input:          structure       -- structure to set min/max in
Outputs:        min             -- value of minimum data point
		max             -- value of maximum data point
Returns:                        -- 1 if successful;  0 if failure
-----------------------------------------------------------------------------*/
extern int  UCDstructure_set_cell_minmax ();



/*---------------------------------------------------------------------------
int UCDstructure_invalid_node_minmax (structure)
UCD_structure        *structure;

This routine allows the module writer to set the min/max range of the
structure node data to be invalid.  This function should be used after 
the structure data has been changed by the module and the module does 
not want to spend the time recomputing the node minmax.                   

Input:          structure       -- structure to set node min/max invalid
Outputs:                        -- none
Returns:                        -- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_invalid_node_minmax ();



/*---------------------------------------------------------------------------
int  UCDstructure_get_node_minmax (structure, min, max)
UCD_structure   *structure;
float           *min;
float           *max;

This routine allows the module writer to obtain the range of the
structure node data.  It should be noted that min and max are arrays of
dimension structure->node_veclen.

Input:          structure       -- structure to get min/max in
Outputs:        min             -- value of minimum data point
		max             -- value of maximum data point
Returns:                        -- 1 = valid data;  0 = invalid data
-----------------------------------------------------------------------------*/
extern int  UCDstructure_get_node_minmax ();



/*---------------------------------------------------------------------------
int  UCDstructure_set_node_minmax (structure, min, max)
UCD_structure   *structure;
float           *min;
float           *max;

This routine allows the module writer to set the range of the
structure node data.  It should be noted that min and max are arrays of
dimension structure->node_veclen.

Input:          structure       -- structure to set min/max in
Outputs:        min             -- value of minimum data point
		max             -- value of maximum data point
Returns:                        -- 1 if successful;  0 if failure
-----------------------------------------------------------------------------*/
extern int  UCDstructure_set_node_minmax ();



/*----------------------------------------------------------------------------
int UCDstructure_get_cell_data (structure, data)
UCD_structure	*structure;
float		**data;     

This function returns a pointer to the array containing the data vectors
for all of the cells in the structure.  
 
Inputs:		structure	- structure to find information
Outputs:	data		- pointer to the cell data vectors
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_cell_data ();
 
 
 
/*----------------------------------------------------------------------------
int UCDstructure_set_cell_data (structure, data)
UCD_structure	*structure;
float		*data;     

This function copies the cell data from the array pointed to by "data"
into the structure's cell data array.  There should be cell_veclen*ncells
data elements in this array.

Inputs:		structure	- structure to find information
        	data		- pointer to the cell data vectors
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_cell_data ();
 
 
 
/*----------------------------------------------------------------------------
int UCDstructure_get_node_data (structure, data)
UCD_structure	*structure;
float		**data;     

This function returns pointers to the array containing the data vectors
for the nodes.

Inputs:		structure	- structure to find information
Outputs:	data		- pointer to the node data vectors
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_data ();


 
/*----------------------------------------------------------------------------
int UCDstructure_set_node_data (structure, data)
UCD_structure	*structure;
float		*data;     

This function copies the node data from the array pointed to by "data"
into the structure's node data array.  There should be node_veclen*nnodes
data elements in this array.

Inputs:		structure	- structure to find information
        	data		- pointer to the node data vectors
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_data ();



/*----------------------------------------------------------------------------
int UCDstructure_get_cell_components (structure, components)
UCD_structure	*structure;
int  		**components;     

This function returns pointers to the array containing the cell component
list.  For instance, if there are four different components in the
cell data vector (e.g.  scalar, 3-vector, 2-vector, scalar), the component
list would be:  (1 3 2 1)

Inputs:		structure	- structure to find information
Outputs:	components	- pointer to the cell component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_cell_components ();


 
/*----------------------------------------------------------------------------
int UCDstructure_set_cell_components (structure, components, number)
UCD_structure	*structure;
int  		*components;     
int		number;

This function copies the array containing the cell component
list.  For instance, if there are four different components in the
cell data vector (e.g.  scalar, 3-vector, 2-vector, scalar), the component
list would be:  (1 3 2 1)

Inputs:		structure	- structure to find information
         	components	- pointer to the cell component list  
		number		- number of components in the list
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_cell_components ();


 
/*----------------------------------------------------------------------------
int UCDstructure_get_node_components (structure, components)
UCD_structure	*structure;
int  		**components;     

This function returns pointers to the array containing the node component
list.  For instance, if there are four different components in the
node data vector (e.g.  scalar, 3-vector, 2-vector, scalar), the component
list would be:  (1 3 2 1)

Inputs:		structure	- structure to find information
Outputs:	components	- pointer to the node component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_components ();


 
/*----------------------------------------------------------------------------
int UCDstructure_set_node_components (structure, components, number)
UCD_structure	*structure;
int  		*components;     
int		number;

This function copies the array containing the node component
list.  For instance, if there are four different components in the
node data vector (e.g.  scalar, 3-vector, 2-vector, scalar), the component
list would be:  (1 3 2 1)

Inputs:		structure	- structure to find information
         	components	- pointer to the node component list  
		number		- number of components in the list
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_components ();



/*----------------------------------------------------------------------------
int UCDstructure_get_cell_active (structure, active)
UCD_structure	*structure;
int  		**active;     

This function returns a pointer to the array containing the cell 
active component list.  For instance, if there are four different components 
in the cell data vector and the module is using the second component, the
list would be:  (0 1 0 0)

Inputs:		structure	- structure to find information
Outputs:	active		- pointer to the cell active component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_cell_active ();


 
/*----------------------------------------------------------------------------
int UCDstructure_set_cell_active (structure, active)
UCD_structure	*structure;
int  		*active;     

This function sets the array containing the cell active component list.  
For instance, if there are four different components in the cell data 
vector and the module is using the second component, the list would 
be:  (0 1 0 0)

Inputs:		structure	- structure to find information
         	active		- pointer to the cell active component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_cell_active ();


 
/*----------------------------------------------------------------------------
int UCDstructure_get_node_active (structure, active)
UCD_structure	*structure;
int  		**active;     

This function returns a pointer to the array containing the node 
active component list.  For instance, if there are four different components 
in the node data vector and the module is using the second component, the
list would be:  (0 1 0 0)

Inputs:		structure	- structure to find information
Outputs:	active		- pointer to the node active component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_active ();


 
/*----------------------------------------------------------------------------
int UCDstructure_set_node_active (structure, active)
UCD_structure	*structure;
int  		*active;     

This function sets the array containing the node active component list.  
For instance, if there are four different components in the node data 
vector and the module is using the second component, the list would 
be:  (0 1 0 0)

Inputs:		structure	- structure to find information
         	active		- pointer to the node active component list  
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_active ();


 
/*----------------------------------------------------------------------------
int UCDstructure_get_node_positions (structure, x, y, z)
UCD_structure	*structure;
float		**x, **y, **z;

This function returns pointers to the arrays containing the x, y and z
coordinates of node positions.

Inputs:		structure	- structure to find information
Outputs:	x, y, z		- pointer to the x,y,z arrays
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_get_node_positions ();



/*----------------------------------------------------------------------------
int UCDstructure_set_node_positions (structure, x, y, z)
UCD_structure	*structure;
float		*x, *y, *z;

This function copies the x, y and z coordinate arrays from the arrays
pointed to by "x", "y" and "z" into the structure's node position arrays.
There should be nnodes coordinates in each array.

Inputs:		structure	- structure to find information
        	x, y, z		- pointer to the x,y,z arrays
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDstructure_set_node_positions ();


 

/*----------------------------------------------------------------------------
int UCDcell_get_information (structure, cell, name, element_type, 
			     material_type, cell_type, 
 			     mid_edge_flags, node_list)
UCD_structure	*structure;
int		cell;
int 		*name;
char		*element_type;
int 		*material_type;
int		*cell_type;
int		*mid_edge_flags;
int		**node_list;

This function finds out all the information about a particular cell and
returns those values.

Inputs:		structure	- structure to find information
		cell		- cell to find information
Outputs:	name		- cell name
		element_type	- name of element type
		material_type	- user defined material type
		cell_type	- cell type (e.g. UCD_TRIANGLE)
		data		- data for cell-based datasets
		mid_edge_flags	- does the cell have mid edge nodes
		node_list	- array of node numbers
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDcell_get_information ();



/*----------------------------------------------------------------------------
int UCDcell_set_information (structure, cell, name, element_type, 
			     material_type, cell_type, 
 			     mid_edge_flags, node_list)
UCD_structure	*structure;
int		cell;
int 		name;
char		*element_type;
int 		material_type;
int		cell_type;
int		mid_edge_flags;
int		*node_list;

This function sets all the information about a particular cell.

Inputs:		structure	- structure to find information
		cell		- cell to find information
		name		- cell name
		element_type	- name of element type
		material_type	- user defined material type
		cell_type	- cell type (e.g. UCD_TRIANGLE)
		mid_edge_flags	- does the cell have mid edge nodes
		node_list	- array of node numbers
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDcell_set_information ();



/*----------------------------------------------------------------------------
int UCDnode_get_information (structure, node, name, ncells, cell_list)
UCD_structure	*structure;
int		node;
int 		*name;
int		*ncells;
int		**cell_list;

This function finds out all the information about a particular node and
returns those values.

Inputs:		structure	- structure to find information
		node		- node to find information
Outputs:	name		- node name
		ncells		- number of cells in cell_list
		cell_list	- array of cell numbers
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDnode_get_information ();


 
/*----------------------------------------------------------------------------
int UCDnode_set_information (structure, node, name, ncells, cell_list)
UCD_structure	*structure;
int		node;
int 		name;
int		ncells;
int		*cell_list;

This function sets all the information about a particular node.

Inputs:	        structure	- structure to find information
		node		- node to find information
		name		- node name
		ncells		- number of cells in cell_list
		cell_list	- array of cell numbers
Returns:			- 1 if successful, 0 if failure
-----------------------------------------------------------------------------*/
extern int UCDnode_set_information ();


/*  end ucd_defs.h  */
