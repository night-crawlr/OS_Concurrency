#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<pthread.h>


int no_company,no_vaccen,no_stude;
long double chance[1005];
pthread_t ctid[1005];
pthread_t vtid[1005];
pthread_t stid[1005];
int finalized=0;
int waiting=0 ;
typedef struct iterr
{
    int ind;
}iterr;

typedef struct batch
{
    int comid;
    int batchno;
    long double prob;
    int vacc;
}batch;
typedef struct company
{
    int id;
    int sec;
    long double pro;
    batch batches[10];
    int no_batches;
}company;
typedef struct center
{
    int id;
    batch* dabba;
    int slots;
    int dup_slots;
}center;
typedef struct student
{
    int id;
    int is_waiting;
    int is_finalized;
    int round;
    int is_injected;
    int vacid; // id of vaccination centre the student if he has booked a slot otherwise it is 0 
}student;
company c[1005];
center v[1005];
student s[1005];
pthread_mutex_t c_lock[1005],v_lock[1005],s_lock[1005];
pthread_mutex_t lockc,lockv,locks;
int is_vaccines_completed(int id,int no_batches)
{
    for(int i =1 ;i<= no_batches;i++)
    {
        if(c[id].batches[i].vacc!=0) return 0;
    }
    return 1;
}
int random_time( void )
{
    return(2 + (rand()%4));
}
int random_batch( void )
{
    return(1 + (rand()%5));
}
int random_vaccines( void )
{
    return(10 + (rand()%11));
}
int random_student( void )
{
    return(2 + (rand()%3));
}
int random_toss(long double p)
{
   // printf("\nIAM IN RANDOM TOSS\n");
    p=p*100;
    int p1= (int)p;
    int numberchoose = (rand()%100) +1;
    if(numberchoose<=p1)
        return 1;
    return 0;
}
int random_slots (int min , int mini1 ,int mini2)
{
    int mini=8;
    if(mini >  mini1)
        mini = mini1;
    if(mini > mini2)
        mini = mini2;
    int ans;
    if(mini == 0)
        ans=0;
    else
       ans =  min + ( rand()%(mini-min+1)); 
    return ans;
}
void * s_thread(void * s1)
{
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
}
void * v_thread(void * v1)
{   
    center* new = (center*) v1; 
    int vid= new->id;
    if(finalized==no_stude)
        pthread_exit(NULL);
    if(v[vid].dabba->vacc==0)
    {
        v[vid].dabba=NULL;
        printf("\n\033[1;36mVACCINATION ZONE %d HAS RUN OUT OF VACCINES\n",vid);
        pthread_exit(NULL);
    }   
// here a student lock is needed because only solts transfering or updating of slota should happen so only either this or student pdatiion should happen
    pthread_mutex_lock(&v_lock[vid]);
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
    pthread_mutex_unlock(&v_lock[vid]);
   // printf("(%d %d)",)
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
}
void * c_thread(void* c1)
{
    int count=0;
 //  printf("\n(((((((%d))))(1)))\n",count++);
    company* new = (company*) c1;
   // printf("\n(((((((%d)))(2))\n",count++);
    int index=new->id;
   // printf("\n(((((((%d)))3))\n",count++);
    if(finalized==no_stude)
    {
        pthread_exit(NULL);
    //printf("\n(((((((%d)))4))\n",count++);
    }
    //printf("\n(((((((%d))5)))\n",count++);
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
    //printf("\n(((((((%d))))8)\n",count++);
    printf("\n\033[1;34mPHARMACEUTICAL COMPANY %d HAS PREPARED %d BATCHES OF VACCINES WHICH HAVE SUCCESS PROBABILITY %LF\n",index,no_batches,prob);
    //printf("\n(((((((%d))))9)\n",count++);

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
    while( (is_vaccines_completed(index,no_batches) == 0) && (finalized != no_stude) ); // break if all students are finalized or batches got emptiedd
    //printf("\n(((((((%d)))13))\n",count++);
    
    if(is_vaccines_completed(index,no_batches)==1) 
    {
        printf("\n\033[1;34mALL VACCINES PRODUCED BY COMPANY %d ARE USED UP SO RESUMING PRODUCTION\n",index);
    }
    //printf("\n(((((((%d)))14))\n",count++);
    c_thread(c1);
    //printf("\n(((((((%d)))15))\n",count++);
}
int main()
{
    pthread_mutex_init(&lockc,NULL);
    pthread_mutex_init(&lockv,NULL);
    pthread_mutex_init(&locks,NULL);
    for(int i=0;i<1005;i++)
    {
        pthread_mutex_init(&c_lock[i],NULL);
        pthread_mutex_init(&v_lock[i],NULL);
        pthread_mutex_init(&s_lock[i],NULL);
    }
    printf("\n\033[1;33mENTER NO OF COMAPANIES : ");
    scanf("%d",&no_company);
    printf("\n\033[1;33mENTER NO OF VACCINATION CENTERS : ");
    scanf("%d",&no_vaccen);
    printf("\n\033[1;33mENTER NO OF STUDENTS : ");
    scanf("%d",&no_stude);
    c[0].id=100;
    for(int i=1;i<=no_company;i++)
    {
        c[i].id=i;
        c[i].no_batches=0;
        c[i].pro=chance[i];
        c[i].sec=0;
        printf("\n\033[1;33mENTER THE PROBABILITY OF SUCCESS OF COMPANY - %d VACCINE : ",i);
        scanf("%LF",chance + i);
    }
    for(int i=1;i<=no_vaccen;i++)
    {
        v[i].id=i;
        v[i].dabba=NULL;
        v[i].slots = 0;
        v[i].dup_slots=0;
    }
    s[0].id=101;
    for(int i =0;i<1005;i++)
    {
        s[i].id=i;
        s[i].is_finalized=0; // whther he is completed his purpose or not
        s[i].is_waiting=0; // waiting becomes 1 when he came to collage
        s[i].is_injected=0;  // if he is succesfully injected antibodies
        s[i].round=0; // round is upadated when he goes for vaccination 
        s[i].vacid=0;
    }
    if(no_company == 0)
    {
        printf("\n\033[1;33mNO PHARMACEUTICAL COMAPNIES ARE AVAILABE\n");
        goto ayipaye;
    }
    if(no_vaccen == 0)
    {
        printf("\n\033[1;33mNO VACCINATION CENTERS ARE AVAILABE\n");
        goto ayipaye;
    }
    if(no_stude == 0)
    {
        printf("\n\033[1;33mNO STUDENTS ARE AVAILABE\n");
        goto ayipaye;
    }
    for(int i=1 ;i<=no_company;i++)
    {
        pthread_create(&ctid[i],NULL,c_thread,(void*)&c[i]);
    }
    int studentid=0;
    while(studentid!=no_stude)
    {
        sleep(random_time()); // students are not entering the gate
        int studentscame = 2;
        for(int i=1;i<=studentscame && studentid< no_stude ;i++)
        {
            studentid++;
            iterr id;
            id.ind=studentid;
           // printf("(%d)",studentid);
            pthread_create(&stid[studentid],NULL,s_thread,(void*)&s[studentid]);
            //printf("(%d)",studentid);
        }
        if(studentid == no_stude)
            break;
    }
    /*for(int i=1;i<=no_company;i++)
        pthread_join(ctid[i],NULL);
    for(int i=1;i<=no_vaccen;i++)
        pthread_join(vtid[i],NULL);*/
    for(int i=1;i<=no_stude;i++)
        pthread_join(stid[i],NULL);
    ayipaye:
    printf("\n\033[1;33mSIMULATION COMPLETED\n");
    pthread_mutex_destroy(&lockc);
    pthread_mutex_destroy(&lockv);
    pthread_mutex_destroy(&locks);
    for(int i=0; i<1005; i++)
    {
		pthread_mutex_destroy(&(c_lock[i]));
        pthread_mutex_destroy(&v_lock[i]);
        pthread_mutex_destroy(&s_lock[i]);
    }
    return 0;
}