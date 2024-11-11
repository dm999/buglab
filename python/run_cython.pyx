import numpy as np
cimport numpy as cnp

from libc.stdlib cimport free
from cpython cimport PyObject, Py_INCREF

cnp.import_array()

cdef extern from "cfunc.cpp":
    cnp.uint64_t *compute(int batch, int taskSize, cnp.uint8_t* task, int threads)
    
cdef extern from "genetics.cpp":
    void gen_population(cnp.uint64_t* rewards)
    cnp.uint64_t *get_population()
    void loadState()

#https://gist.github.com/phaustin/4973792
#https://stackoverflow.com/questions/3046305/simple-wrapping-of-c-code-with-cython
cdef class ArrayWrapper:
    cdef void* data_ptr
    cdef int size
    cdef int batch
    cdef int taskSize

    cdef set_data(self, int batch, int taskSize, void* data_ptr):
        """ Set the data of the array
        This cannot be done in the constructor as it must recieve C-level
        arguments.
        Parameters:
        -----------
        size: int
            Length of the array.
        data_ptr: void*
            Pointer to the data
        """
        self.data_ptr = data_ptr
        self.batch = batch
        self.taskSize = taskSize
        self.size = batch * taskSize

    def __array__(self):
        """ Here we use the __array__ method, that is called when numpy
            tries to get an array from the object."""
            
        cdef cnp.npy_intp shape[2]
        shape[0] = <cnp.npy_intp> self.batch
        shape[1] = <cnp.npy_intp> self.taskSize
        
        # Create a 1D array, of length 'size'
        ndarray = cnp.PyArray_SimpleNewFromData(2, shape, cnp.NPY_UINT64, self.data_ptr)
        return ndarray

    def __dealloc__(self):
        """ Frees the array. This is called by Python when all the
        references to the object are gone. """
        free(<void*>self.data_ptr)

def py_compute(cnp.ndarray[cnp.uint8_t, ndim = 2] task, int threads):
    """ Python binding of the 'compute' function in 'c_code.c' that does
        not copy the data allocated in C.
    """
    cdef cnp.uint64_t *array
    cdef cnp.ndarray ndarray
    
    assert task.shape[1] == 551
    
    # Call the C function
    array = compute(task.shape[0], task.shape[1], <cnp.uint8_t*> task.data, threads)

    array_wrapper = ArrayWrapper()
    array_wrapper.set_data(task.shape[0], task.shape[1] + 1, <void*> array) #reward + heatmap
    ndarray = np.array(array_wrapper, copy=False)
    
    # Assign our object to the 'base' of the ndarray object
    #ndarray.base = <PyObject*> array_wrapper
    cnp.PyArray_SetBaseObject(ndarray, array_wrapper) 
    
    # Increment the reference count, as the above assignement was done in
    # C, and Python does not know that there is this additional reference
    Py_INCREF(array_wrapper)


    return ndarray


def py_load_state():
    loadState()

def py_gen_population(cnp.ndarray[cnp.uint64_t, ndim = 1] rewards):
    gen_population(<cnp.uint64_t*> rewards.data)

    
def py_get_population():
    cdef cnp.uint64_t *array
    cdef cnp.ndarray ndarray
    
    array = get_population()
    
    array_wrapper = ArrayWrapper()
    array_wrapper.set_data(1000, 551, <void*> array) 
    ndarray = np.array(array_wrapper, copy=False)
    
    cnp.PyArray_SetBaseObject(ndarray, array_wrapper) 

    Py_INCREF(array_wrapper)


    return ndarray