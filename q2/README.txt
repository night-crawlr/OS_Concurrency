QUESTION 2

COMPILATION COMMAND :
gcc q2.c -lpthread;

STRUCTURES:

1)
typedef struct batch
{
    int comid;
    int batchno;
    long double prob;
    int vacc;
}batch;

Structure batch contains inforatiom of a batch like from hich company it is sent and its id and the no of vaccines in it and its probabaility of comapny

2)
typedef struct company
{
    int id;
    int sec;
    long double pro;
    batch batches[10];
    int no_batches;
}company;

This contains details of company ie its id,probability,no of bacthes it produces and what they are in batches array.

3)
typedef struct center
{
    int id;
    batch* dabba;
    int slots;
    int dup_slots;
}center;

This contains deatails of a vaccination centre ie which batch it currently has otherwaise dabba=NULL and how many slots it has been allocated in each phase this may get decremented by students so dup_slots is duplicate value of real slots value.

4)
typedef struct student
{
    int id;
    int is_waiting;
    int is_finalized;
    int round;
    int is_injected;
    int vacid; // id of vaccination centre the student if he has booked a slot otherwise it is 0 
}student;

Details of a particular student ie his id is he waiting or not is he finalized or not ,is he injected succesfully or not,in which round he is currently to test,his vaccination center if he is not in vacccination center vacid = 0 ; because iam not using '0' as any vaccination centre id

BASIC FUNCTIONS:

1)int is_vaccines_completed(int id,int no_batches)  ---------> This tells company that its vaccines in all bacthes are completed or not

2)int random_time( void ) -------------------------------------> generate random number from (2,5);
3)int random_batch( void ) ------------------------------------> generate random number from (1,5);
4)int random_vaccines( void )----------------------------------> generate random number from (10,20);
5)int random_student( void ) ----------------------------------> generate random number from (2,4);
6)int random_toss(long double p)-------------------------------> decide whether antibodies are injected or not;
7)int random_slots (int min , int mini1 ,int mini2)------------> min is 1 min1 is no of vaccines and min2 is no of currently waiting student this ouput no of slots from 1 to min(8,min1,min2)


LOGIC AND EXPLANATION OF OTHER FUNCTIONS:

1) I have declared a mutex per comapny,vaccenter,student but i realized to not use all of them and i declare another 3 lockc,lockv,locks for restricting several companies on Critical Section by lockc,for restricting several vaccination centers by lockv,several process and stuent by locks.

2)at start comapny threads get created

first it will fill  batches

*****************************************************
    
int sec,no_batches;
    sec = random_time();
    long double prob = chance[index];
    no_batches = random_batch();
    //printf("\n(((((((%d)))6))\n",count++);
    printf("\n\033[1;34mPHARMACEUTICAL COMPANY %d IS PREPARING %d BATCHES OF VACCINES WHICH HAVE SUCCESS PROBABILITY %LF\n",index,no_batches,prob);
    sleep(sec);
    //printf("\n(((((((%d))))7)\n",count++);
    c[index].pro=prob;
    c[index].sec=sec;
    c[index].no_batches=no_batches;
    for(int i = 1; i<= no_batches ;i++)
    {
        c[index].batches[i].prob=prob;
        c[index].batches[i].vacc=random_vaccines();
        c[index].batches[i].batchno=i;
        c[index].batches[i].comid=index;
    }

****************************************************   I will explain mutexes at last *******************


Then it repaeatedly checke untill its bacthes are completed,theese are completed by sending bacthes to empty vaccination centers

***********************************************************
 while(c[index].no_batches>0)
    {
        if(finalized==no_stude)
        {
            break;
        }
    //printf("\n(((((((%d))))10)\n",count++);
       // sleep(5);
        for(int i=1;i<=no_vaccen;i++)
        {
            if(finalized==no_stude)
            {
                break;
            }
            if(c[index].no_batches==0)
            {
                break;
            }
            // mutex is used
    ///printf("\n(((((((%d)))11))\n",count++);
    //sleep(5);
            if(pthread_mutex_trylock(&v_lock[i])==0) // EXTRA MODIFICATION
            {
                if(v[i].dabba==NULL) // if a center is empty
                {
                    printf("\n\033[1;34mPHARMACEUTICAL COMPANY %d IS DELIVERING VACCINES TO VACCINATION ZONE %d WHICH HAS SUCCESS PROBABAILITY %LF WITH BATCHNO (%d)\n",index,i,c[index].pro,c[index].no_batches);
                    v[i].dabba=&(c[index].batches[c[index].no_batches]);
                    c[index].no_batches--;
                    printf("\n\033[1;34mPHARMACEUTICAL COMPANY %d HAS DELIVERED %d VACCINES TO VACCINATION ZONE %d, RESUMING VACCINATIONS NOW WITH BATCHNO (%d) \n",index,v[i].dabba->vacc,i,v[i].dabba->batchno);
                    pthread_mutex_unlock(&v_lock[i]);
                    iterr id;
                    id.ind=i;
    //printf("\n(((((((%d)))11.5))\n",count++);
        //            sleep(5);
                    pthread_create(&vtid[i],NULL,v_thread,(void*)&v[i]);
                    continue;
                }
                pthread_mutex_unlock(&v_lock[i]);
            }
    //printf("\n(((((((%d)))12))\n",count++);
      //  sleep(5);
        }
    }
*********************************************************************************************

after suuplying batches to vaccination centers it waits untill its all vaccines are used up or wait untill all students are finalized ie either 3rd negative or anyround positive,then it exits for prodcing another finiete no of batches 

******************************************************************************************

while( (is_vaccines_completed(index,no_batches) == 0) && (finalized != no_stude) ); // break if all students are finalized or batches got emptiedd
    //printf("\n(((((((%d)))13))\n",count++);
    
    if(is_vaccines_completed(index,no_batches)==1) 
    {
        printf("\n\033[1;34mALL VACCINES PRODUCED BY COMPANY %d ARE USED UP SO RESUMING PRODUCTION\n",index);
    }
    //printf("\n(((((((%d)))14))\n",count++);
    c_thread(c1);
    //printf("\n(((((((%d)))15))\n",count++);
**************************************************************************************
This is company therad
Above you can see when a bacth is delivered to vac_cen then its corresponding vac_ccen is invoked

3)expaining vac_cen thread

Now this vac_cen tries to create slots so it waits untill there are some stuednts ie waiting(global variable used to specify no of waitng students at an instant) is non zero.
and then it creates slots mostly slots value is > 0 but for care i wrote if slots is 0 then exeucte  next phase ie by recurcivley calling again


************************************************************************************

while (waiting==0);
    sleep(5); //sleeping beacause when waiting value becomes no zero it becomes 1 probabaly and at nxt step for creating slots then mostly slot number become 1 or 2 ie small no of slots so if we wait for 2 secc then may be we can get more waitng memebers
    v[vid].slots=random_slots(1,v[vid].dabba->vacc,waiting);
    v[vid].dup_slots=v[vid].slots;    
    if(v[vid].slots==0)
    {
        pthread_mutex_unlock(&v_lock[vid]);
        v_thread(v1);
    }
    printf("\n\033[1;36mVACCINATION ZONE %d IS READY TO VACCINATE WITH %d SLOTS\n",vid,v[vid].slots);
    
************************************************************************************
 now this waits untill slots are filled up or waiting is 0 and then vaccination phase starts,after vaccination phase it checks whether all vaccines are used up or not and all stuednts are finalized or not and then recusively call for next vaccination phase if possible

*******
************************************************************************

while(v[vid].slots>0 && waiting>0 ); // wait untill slots are filled up
    pthread_mutex_lock(&v_lock[vid]);  //locking th vaccinatio phase so that once slots are fixed no student should decreent the slots value
    
    int noofstudents= v[vid].dup_slots-v[vid].slots;
    if(noofstudents!=0)
    printf("\n\033[1;36mVACCINATION ZONE %d ENTERING VACCINATION PHASE\n",vid);
    for(int i=1;i<=no_stude;i++)
    {
        if(s[i].vacid!=vid) // checking student is waiting to be vaccinated in vid
            continue;
        v[vid].dabba->vacc--;
        printf("\n\033[1;31mSTUDENT %d ON VACCINATION ZONE %d HAS BEEN VACCINATED WHICH HAS SUCCESS PROBABILITY %LF\n",i,vid,v[vid].dabba->prob);
        //printf("()()()(0");
        int decide = random_toss(v[vid].dabba->prob);
        //printf("(%d)",decide);
        s[i].is_injected=decide;
        s[i].vacid=0;
    }
    v[vid].dup_slots=0;
    v[vid].slots=0;
    pthread_mutex_unlock(&v_lock[vid]);
    if(v[vid].dabba->vacc==0)
    {
        v[vid].dabba=NULL;
        printf("\n\033[1;36mVACCINATION ZONE %d HAS RUN OUT OF VACCINES\n",vid);
        pthread_exit(NULL);
    } 
    if(finalized==no_stude)
        pthread_exit(NULL);
    v_thread(v1);

*********************************************************************************
                                                                 ****************
4) Explaining student thread

student come and waits then his round and waiting falg is updated he checks for slots untill he will be allocated with one ,once he has been allocated after vaccination phase that therad will exit by modifying "finalized " and "waiting" and other neccessary variables.



*******************
*********************************************************************************

student* new = (student*) s1; 
    int sid = new->id;
    //printf("%d %d %d %d %d %d",sid,sid,sid,new->id,new->id,new->id);
    s[sid].round++;
    pthread_mutex_lock(&locks);
    waiting++;
    s[sid].is_waiting=1;
    printf("\n\033[1;31mSTUDENT %d HAS ARRIVED FOR ROUND - %d OF VACCINATION\n",sid,s[sid].round);
    pthread_mutex_unlock(&locks);
    //now start searching for slots in each vaccination centre
    printf("\n\033[1;31mSTUDENT %d IS WAITING TO BE ALLOCATED A SLOT ON A VACCINATION ZONE\n",sid);
    while(1)
    {
        for(int i=1;i<=no_vaccen;i++)
        {
            if(pthread_mutex_trylock(&v_lock[i])==0) // EXTRA MODIFICATION IS NEEDED MUST
            {
                if(v[i].slots == 0)
                {
                    pthread_mutex_unlock(&v_lock[i]);
                    continue;
                }
                v[i].slots--;
                s[sid].vacid=i;
                s[sid].is_waiting=0;
                waiting--;
                printf("\n\033[1;31mSTUDENT %d ASSIGNED A SLOT ON THE VACCINATION ZONE %d AND WAITING TO BE VACCINATED\n",sid,i);
                pthread_mutex_unlock(&v_lock[i]);
                while(s[sid].vacid!=0); // he is still under some vaccination centre
                pthread_mutex_lock(&locks);
                if(s[sid].is_injected == 0)
                {
                    printf("\n\033[1;31mSTUDENT %d HAS TESTED 'NEGATIVE’ FOR ANTIBODIES\n",sid);
                    if(s[sid].round == 3)
                    {
                        s[sid].is_finalized=1;
                        finalized++;
                        pthread_mutex_unlock(&locks);
                        pthread_exit(NULL);
                    }
                    s[sid].is_finalized=0;
                    s[sid].is_waiting=1;
                    pthread_mutex_unlock(&locks);
                    s_thread(s1);
                }
                else
                {
                    printf("\n\033[1;31mSTUDENT %d HAS TESTED ‘POSITIVE’ FOR ANTIBODIES\n",sid);
                    s[sid].is_finalized=1;
                    finalized++;
                    s[sid].is_waiting=0;
                    pthread_mutex_unlock(&locks);
                    pthread_exit(NULL);
                }
            }
        }
    }


**********************************************************************************************
										**************



5) EXPLAING MUTEX LOCKS

line 110 pthread_mutex_lock(&locks);
this makes the waiting++ doesnot fall into race condition by many students
and unlocks at line 114


line 121 pthread_mutex_trylock(&v_lock[i]);
this is cooperating with mutex lock at line 177 beacause when slots are producing no student should check or update ,so here mutal exclusion matters so by this we can setablish mutual exclusion ie either upadting slots or genreation of slots happen at an instant.this locks are relased at end of line 133 and line 188 respectively.


after that mutex lock in students thread another locks mutex is used in line 135 beacause there involve updatation of global variables finalized++,waiting++ so no other student should acces at same time so mutex is used


in line 191 mutexlock v_lock[vid] is used beacause here it is entering vacciantion pahse so no other student should update or acces the slots of that vid this lock will cooporate with above mutextry lock making no student can accens slots when vac_cen is in vaccinatiing pahse this mutex unlocks in line 210


inline 278 mutex lock v_lock is used as no 2 compnanie should check for no of batches in same vaccination centere this mutex unlocks at line 286


6) at last we join threads of students.
