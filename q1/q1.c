#include<stdio.h>
#include<time.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<wait.h>
#include<limits.h>
#include<fcntl.h>
#include<pthread.h>
#include<inttypes.h>
#include<math.h>
long long int N;
int arr[100005];
int arr2[100005];
int * sharr;
typedef struct s_arr{
    long long int start;
    long long int last;
    int* arr;
}s_arr;
void *cre_shm(size_t size)
{
    int shm_id = shmget(IPC_PRIVATE,size,0666 | IPC_CREAT);
    return shmat(shm_id,NULL,0);
} 
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
int main()
{
    s_arr thread_arr;
    struct timespec ts;
    
    printf("ENTER THE NUMBER OF  NON NEGATIVE INETEGERS TO BE SORTED : ");
    scanf("%lld",&N);
    printf("ENTER %lld INTEGERS : ",N);
    for(long long int i=0;i<N;i++)
    {
        scanf("%d",arr+i);
    }
    sharr = (int *)cre_shm(sizeof(int) * (N+1));
    for(long long int i=0;i<N;i++)
    {
        sharr[i]=arr[i];
        arr2[i]=arr[i];
    }
    (&thread_arr)->start = 0;
    (&thread_arr)->last = N-1;
    (&thread_arr)->arr = arr2;
    printf("\n-----------------CONCURRENT MERGE SORT---------------\n");
    // DOING CONCURRENT_MERGE
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
    long double begin = ts.tv_nsec/(1e9)+ts.tv_sec;
    concurrent_merge(sharr,0,N-1);
    for(long long int i=0;i<N;i++)
    {
        printf("%d ",sharr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
    long double end = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double t_concurrent = (double) (end-begin);
    printf("Time taken for CONCURRENT MERGESORT  = %Lf sec's\n",t_concurrent);

    printf("\n-----------------NORMAL MERGE SORT---------------\n");
    // NORMAL MERGE
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
    begin = ts.tv_nsec/(1e9)+ts.tv_sec;
    normal_merge(arr,0,N-1);
    for(long long int i=0;i<N;i++)
    {
        printf("%d ",arr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
     end = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double t_normal = (double) (end - begin)  ;
    printf("Time taken for NORMAL MERGESORT = %Lf sec's\n\n",t_normal);

    printf("\n-----------------THREDED MERGE SORT--------------\n");
    //threaded merge sort
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
    begin = ts.tv_nsec/(1e9)+ts.tv_sec;
    pthread_t tid;
    pthread_create(&tid,NULL,thread_merge,&thread_arr);
    pthread_join(tid,NULL);
    for(long long int i=0;i<N;i++)
    {   
        printf("%d ",arr2[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
     end = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double t_thread = (double) (end - begin) ;
    printf("Time taken for THREAD MERGESORT = %Lf sec's\n\n",t_thread);

    printf("NORMAL MERGE IS %Lf times faster than CONCURRENT MERGE and %Lf times fater than THREADED MERGE \n",t_concurrent/t_normal,t_thread/t_normal);
    shmdt(sharr);
}