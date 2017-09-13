#include <stdlib.h>
#include <sys/types.h>
#include <vector>
#include <sys/stat.h> 
#include <CL/cl.h>
#include <iostream>
#include <time.h>
#define WIDTH 213
#define HEIGHT 213
using namespace std;
int main()
{	
	freopen("in.txt","r",stdin); //open a file for reading purpsoe
	cl_platform_id platform;     //Opencl Plateform
	cl_context context;          //context
	cl_command_queue queue;      //command queue
	cl_device_id device;         //device id
	cl_int error;
	
	if (clGetPlatformIDs(1,&platform, NULL) != CL_SUCCESS) 
	{

//platform
//1st-number of cl_platform_id entries that can be added to platforms
//2nd-Returns a list of OpenCL platforms found. The cl_platform_id values returned in platforms can be //used to identify a specific OpenCL platform.
//3rd-Returns the number of OpenCL platforms available 
	
	std::cout << "Error getting platform id\n";
	exit(error);
	}

	std::cout << "PLatform Id :" << platform <<"\n";

	// Device
	if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL)!= CL_SUCCESS) 
	{

//device
//1st- plateform id return by clGetPlateformsIDs
//2nd-identifies the type of OpenCL device
//3rd-number of cl_device entries that can be added to devices
//4th-value fill by clGetDeviceIDs into device which is device id
//5th-number of OpenCL devices available that match device_type. If num_devices is NULL, this 
//argument is ignored.

	std::cout << "Error getting device ids\n";
	exit(error);
	}
	std::cout << "Device Id :" << device <<"\n";

	// Context
	context = clCreateContext(0, 1, &device, NULL, NULL, &error);

//1st-Specifies a list of context.
//2nd-number of devices specified in the devices argument.
//3rd-A pointer to a list of unique devices returned by clGetDeviceIDs for a platform.
//4th-A callback function that can be registered by the application.If pfn_notify is NULL, no callback //function is registered.This callback function will be used by the OpenCL implementation to report //information on errors that occur in this context. This callback function may be called asynchronously //by the OpenCL implementation. It is the application's responsibility to ensure that the callback //function is thread-safe.
//5th-Passed as the user_data argument when pfn_notify is called. user_data can be NULL.
//6th-Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.

	if (error != CL_SUCCESS) 
	{
	    std::cout << "Error creating context\n";
	    exit(error);
	}
	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &error);  
//command-queue
//1st-Must be a valid OpenCL context
//2nd-Must be a device associated with context. It can either be in the list of devices specified when //context is created using clCreateContext or have the same device type as the device type specified //when the context is created using clCreateContextFromType.
//3rd-Specifies a list of properties for the command-queue. This is a bit-field.
//4th-Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.

//value of CL_SUCCESS is 0 by default.

	if (error != CL_SUCCESS) 
	{
	std::cout << "Error creating command queue\n";
	exit(error);
	}

	
	// Initializing the input matrix
	int ipMat[WIDTH][HEIGHT] ;
	for(int i=0; i<HEIGHT; i++)
		for(int j=0; j<WIDTH; j++)
		{
			std::cin >> ipMat[j][i];
		}
	
		// Initializing the output matrix with zeros
	/*int opMat[WIDTH][HEIGHT];*/

	// computing total size of image's matrix

	const int totalImgSize = (HEIGHT)*(WIDTH);
	std::cout << "This is a break point to calculate image size :" << totalImgSize << "\n";

	// Computing the size of buffer needed to store the image(matrix) data
	const int buffSize = sizeof(int)*totalImgSize;

	//dynamically creating 1D matrix (Arrys) to store input and output 2D matrix

	int* ipArray = new int[totalImgSize];
	int* opArray = new int[totalImgSize];
	
	//Storing input matrix to input 1D array and finding Maximum and Minimum
	int p = 0;
	int min=ipMat[0][0],
		max=ipMat[WIDTH-1][HEIGHT-1];

	for(int i=0; i<HEIGHT; i++)
	{
		for(int j=0; j<WIDTH; j++)
		{
			ipArray[p]=ipMat[i][j];
			if (min > ipArray[p])
				min=ipArray[p];
			if (max < ipArray[p])
				max=ipArray[p];
			p++;
		}
	}

	std::cout << "minimum value is :" << min <<"\n";
	std::cout << "mxaimum value is :" << max <<"\n";

	//Creating pointers to values (Only pointers can be passed while setting kernel arguments)
	int *minp = &min;
	int *maxp = &max;
	
	//creating three buffers, two for the same image and one for the output
	cl_mem ipBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, buffSize, ipArray, &error);
	cl_mem opBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffSize, NULL, &error);
	
//1st-valid OpenCL context used to create the buffer object
//2nd-A bit-field that is used to specify allocation and usage information such as the memory arena that //should be used to allocate the buffer object and how it will be used.

	//Creating program from reading the .cl file
	cl_program program;
	FILE *program_handle;
	char* program_buffer;
	size_t program_size;

	program_handle = fopen("processMatrix.cl", "r");
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	//std::cout<<program_size<<"\n";
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size,program_handle);
	fclose(program_handle);

	program = clCreateProgramWithSource(context, 1,(const char**)&program_buffer, &program_size, &error);

	// Builds the program
	error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	// Extracting the kernel
	cl_kernel matProc = clCreateKernel(program, "procMat", &error);

	//Passing kernel arguments
	error = clSetKernelArg(matProc, 0, sizeof(cl_mem), &ipBuffer);
	error |= clSetKernelArg(matProc, 1, sizeof(cl_mem), &opBuffer);
	error |= clSetKernelArg(matProc, 2, sizeof(size_t), minp);
	error |= clSetKernelArg(matProc, 3, sizeof(size_t), maxp);

	//Launching kernel
	const size_t work_units_per_kernel = (size_t)totalImgSize;
	clEnqueueNDRangeKernel(queue, matProc, 1, NULL,  &work_units_per_kernel, NULL, 0, NULL, NULL);

	// For time Profiling
	cl_event timing_event;
	cl_ulong time_start, time_end;
	float read_time;
	//reading the output and putting from opBuffer buffer to opArray outputArray
	clEnqueueReadBuffer(queue, opBuffer, CL_TRUE, 0, buffSize, opArray, 0, NULL, &timing_event);
	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_START,
	sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(timing_event, CL_PROFILING_COMMAND_END,
	sizeof(time_end), &time_end, NULL);
	read_time = time_end - time_start;
	printf("..............%f(miliseconds).................\n\n",(read_time/1000000));

	
	//printing the formatted output for input to MATLAB
	freopen("out.txt", "w", stdout);
	int index = 0;
	std::cout << "[";
	for(int i =0; i< HEIGHT; i++){
		for(int j=0; j< WIDTH; j++)
		{
			std::cout << opArray[index] <<",";
			index++;
		}
		std::cout << ";\n";
	}
	std::cout << "]";

	// Cleaning up
	delete[] ipArray;
	delete[] opArray;
	clReleaseKernel(matProc);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	clReleaseMemObject(ipBuffer);
	clReleaseMemObject(opBuffer);
	return 0;
}
