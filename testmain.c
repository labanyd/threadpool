#include <stdio.h>
#include <stdlib.h>

#include "threadpool.h"


PthreadPool *pool;

void func( void *arg)
{
	int i = *(int*)arg;
	while (1) 
	{
	printf("%d >>> %d\n", pthread_self(), i);
	sleep(1);
	}
}


int main(int argc, char **argv)
{

	int a[100] = {0};

	pool = PthreadPoolInit(5);
	
	if (pool == NULL) 
	{
		printf("init error\n");
		exit(1);
	}
	int i ;

	for (i = 0; i < 100; i++)
	{
		a[i] = i;
		PthreadPoolAddTask(pool, &func, (void*)(a + i));
	}	


	sleep(1);

	while (pool->m_iTaskNum != 0) 
	{
		i = pool->m_iPthreadNum - 1
		for (; i >= 0; i--)
		{
			printf("thread:%d  is %d\n", *(pool->m_pth + i), *(pool->m_pStatus + i));
		}
		fflush(stdout);
		sleep(1);
	}


	PthreadPoolDestroy(pool);
	printf("destory over\n");

}
