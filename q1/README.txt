QUESTION 1 CONCURRENT MERGE SORT


COMPILING COMMAND:
 gcc q1.c -lpthread;

STRUCTURES:
	
1)
typedef struct s_arr{
    long long int start;
    long long int last;
    int* arr;
}s_arr;

This structure is used to send argument to threads 

FUNCTIONS:

1)
void *cre_shm(size_t size)
{
    int shm_id = shmget(IPC_PRIVATE,size,0666 | IPC_CREAT);
    return shmat(shm_id,NULL,0);
} 
This creates shared memory of size "size";

2)
void selection(int * arr , long long int start ,long long int last)
{
    //printf("(%lld %lld)",start,last);
    for(long long int i= start ; i<= last ;i++)
    {
        long long int mini = i;
        for(long long int j=i;j<=last ;j++)
        {
            if(arr[mini] > arr[j] )
            {
                mini = j;
            }
        }
        int temp;
        temp = arr[mini];
        arr[mini]=arr[i];
        arr[i]=temp;
    }
}
This function does selection sort for array elements less than 5

3)
void merge(int * arr , long long int start ,long long int mid1 ,long long int mid2,long long int last)
{
    int temp[100005] ;
    long long int pointer1,pointer2;
    pointer1 = start;
    pointer2=mid2;
    long long int index=0;
    
    while (pointer1 <= mid1 && pointer2 <= last)
    {
        if(arr[pointer1] < arr[pointer2])
            temp[index++]= arr[pointer1++];
        else
            temp[index++]= arr[pointer2++];
    }
    while (pointer2 <= last)
    {
        temp[index++]=arr[pointer2++];
    }
    while (pointer1 <= mid1)
    {
        temp[index++] = arr[pointer1++];
    }
    for(pointer1 = start,index =0;pointer1<=last;index++,pointer1++)
        arr[pointer1]=temp[index];
}
This function mergers two sorted arrays,will be used after sorting of two parts of array

4)
void normal_merge(int* arr,long long int start,long long int last)
{
    long long int mid = (last-start)/2;
    mid = mid + start;
    if(last - start < 4)
    {
        selection(arr,start,last);
        return;
    }
    normal_merge(arr,start,mid);
    normal_merge(arr,mid+1,last);
    merge(arr,start,mid,mid+1,last);
}
This function is a normal merge sort used without any involvement of threads and process,this evantually sorts left half and right half and merge these two arrays this is done by recurssion and we can say that the base condition is last - start < 4 ie array elements are less than 5 then apply merge sort

5)
void concurrent_merge(int * arr,long long int start,long long int last)
{    

    int status;
    long long int mid = (last-start)/2;
    mid = mid + start;
    if(last - start < 4 )
    {
        selection(arr,start,last);
        return;       
    }
    pid_t pid1 = fork();
    if(pid1 == 0)
    {
       // printf("child:1 %lld %lld\n",start,mid);
        concurrent_merge(arr,start,mid);
        _exit(1);
    }
    pid_t pid2 =fork();
    if (pid2 == 0)
    {
       // printf("child:2 %lld %lld\n",mid+1,last);
        concurrent_merge(arr,mid+1,last);
        _exit(1);
    }
    waitpid(pid1,&status,0);
    waitpid(pid2,&status,0);
    merge(arr,start,mid,mid+1,last);
    return;
}
This is mergesort that is done by the 2 process each process for each half of an array,and merges them  as shared memory is used. We have to wait untill that two process complete their execution to merge these 2 arrays.

6)
void * thread_merge(void * a)
{
    s_arr * new = (s_arr*)a;
    long long int start = new->start;
    long long int last = new->last;
    int* arr = new->arr;
    long long int mid = (last-start)/2;
    mid = mid + start;
    if (last - start < 4)
    {
        selection(arr,start,last);
        return NULL;
    }
    pthread_t tid1;
    pthread_t tid2;
    s_arr left;
    s_arr right;
    left.start = start;
    left.last = mid;
    left.arr = arr;
    pthread_create(&tid1,NULL,thread_merge,&left);
  
    right.start = mid+1;
    right.last=last;
    right.arr = arr;

    pthread_create(&tid2,NULL,thread_merge,&right);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    merge(arr,start,mid,mid+1,last);
}
This is mergesort that is done bby producing 2 threads of a same process and each part of array is sorted,this can be done as an analogy to 5) function in a way that threads are used instead of process.


LOGIC : 


1)Taking input of specified non negative integers and sharing this array to 2 differnt arrays
2)one array is shared memory between procress this array is used for concurrent_merge
3)first concurrent merge takes place  ,then normal merge and then thread_merge take place at just before start and just after end of each function clock_gettime() is used to note the instant and their substraction gives us time elapsed by each function and these timrs are used to compare which is faster realtively,each sorted array is printed just after completion on function.


REPORT : 

1)Normal merge sort is more faster than concurrent merge and thread merge.on average and for high indexes noramal merge is roughly 100 times faster than concurrent merge and 60 times faster than thread_merge
2) concurent merge is slower than thread merge 
3)This happens beacuse in a normal merge sort second half is sorted only when 1st half is sorted but in concurrent and thread sort both halfes are sorted concurrently this implies the change of memory of one half to other frrequently happen in this two process so there will be lots of cache misses ,but there will be less cache misses in normal merge comapred to these two as only one array until it is sorted is kept in cache.

