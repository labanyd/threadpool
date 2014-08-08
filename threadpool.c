#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "threadpool.h"


PthreadPool * 
PthreadPoolInit(int pthreadNums)
{
	PthreadPool *pool = (PthreadPool *) malloc (sizeof(PthreadPool));
	if (pool == NULL)
	{
		return NULL;
	}


	/* init taskQueue */
	pool->m_pTaskHead = NULL;
	pool->m_pTaskTail = NULL;
	pool->m_iTaskNum = 0;
	

	/* init pthread pool */
	pthread_mutex_init(&pool->m_mutex, NULL);
	pthread_cond_init(&pool->m_cond, NULL);	

	pool->m_pth =  \
		(pthread_t *) malloc (sizeof(pthread_t) * pthreadNums);

	if (NULL == pool->m_pth) 
	{
		free(pool);
		return NULL;
	}

	pool->m_iPthreadNum = pthreadNums;

	pool->m_pStatus = (int *) malloc (sizeof(int) * pthreadNums);
	if (NULL == pool->m_pStatus)
	{
		free(pool->m_pth);
		free(pool);
		return NULL;
	}

	int i , ret;
	PthreadStatus pst;
	for (i = 0; i < pthreadNums; i++) 
	{
		pst.m_pool = pool;
		pst.m_iThread = i;

		ret = pthread_create(pool->m_pth + i, NULL, doPthreadPool,
			(void *)&pst);	

		if (0 != ret) 
		{
			free(pool->m_pStatus);
			free(pool->m_pth);
			free(pool);
			return NULL;
		}	

		*(pool->m_pStatus + i) = THREAD_INIT;

		/* as if the value of pst.m_iThread will change,
			need time to let the doPthreadPool save the
			right value, so need usleep */	
		usleep(1000);
	}

	return pool;
}

void 
FreeTaskQueue(PthreadPool *pool)
{
	TaskElement *ele = pool->m_pTaskHead;

	while (ele) 
	{
		pool->m_pTaskHead = pool->m_pTaskHead->next;
		free(ele);
		ele = pool->m_pTaskHead;
	}

}

int 
PthreadPoolDestroy(PthreadPool *pool)
{
	IF_NULL(pool, PTHREAD_POOL_NULL);


	int i = 0;
	int seq = pool->m_iPthreadNum;
	for (; i < seq; i++) 
	{ 
		pthread_cancel(*(pool->m_pth + i));
		pthread_join(*(pool->m_pth + i), 0);
	}

	free(pool->m_pth);
	free(pool->m_pStatus);
	pthread_mutex_destroy(&pool->m_mutex);
	pthread_cond_destroy(&pool->m_cond);
	
	FreeTaskQueue(pool);
	free(pool);

	return SUCCESS;
}


int 
PthreadPoolAddTask(PthreadPool *pool, Func fun, void *arg)
{

	IF_NULL(pool, PTHREAD_POOL_NULL);
	IF_NULL(fun, TASK_FUNCTION_NULL);

	pthread_mutex_lock(&pool->m_mutex);
	
	TaskElement * ele = (TaskElement *) malloc (sizeof(TaskElement));
	if (NULL == ele) 
	{
		pthread_mutex_unlock(&pool->m_mutex);
		return MALLOC_FAIL;	
	}

	ele->m_pFunc = fun;
	ele->m_pArg = arg;
	ele->next = NULL;


	/* the first task insert */
	if (pool->m_pTaskHead == NULL) 
	{
		pool->m_pTaskHead = ele;
		pool->m_pTaskTail = ele;
	}
	else
	{
		pool->m_pTaskTail->next = ele;	
		pool->m_pTaskTail = ele;
	}

	pool->m_iTaskNum++;

	pthread_cond_broadcast(&pool->m_cond);
	pthread_mutex_unlock(&pool->m_mutex);

	
	return SUCCESS;
}

void 
CleanUp(void *arg)
{
	PthreadPool *pool = ((PthreadStatus *)arg)->m_pool;
	int          seq  = ((PthreadStatus *)arg)->m_iThread;

	pool->m_iPthreadNum--;
        pthread_mutex_unlock(&pool->m_mutex);
        *(pool->m_pStatus + seq) = THREAD_DIE;
}

void *
doPthreadPool(void *arg)
{
	Func fun;
	void *argument; 
	
	PthreadPool *pool = ((PthreadStatus *)arg)->m_pool;	
	int          seq  = ((PthreadStatus *)arg)->m_iThread;

	
	PthreadStatus pStatus = {pool, seq};

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	pthread_cleanup_push(CleanUp, (void*)&pStatus);

	while (1) 
	{
		
		pthread_mutex_lock(&pool->m_mutex);
		
		while (pool->m_iTaskNum == 0)		
		{
			/* may many pthread wait here, if broadcast ,all will be waked
                           , so need while pool->m_iTaskNum == 0. 
			if no do this, will get segement error */
			*(pool->m_pStatus + seq) = THREAD_WAITTING;
			pthread_cond_wait(&pool->m_cond, &pool->m_mutex);	
		}
		
		fun = pool->m_pTaskHead->m_pFunc;
		argument = pool->m_pTaskHead->m_pArg;
		pool->m_pTaskHead = pool->m_pTaskHead->next;
		pool->m_iTaskNum--;


		pthread_mutex_unlock(&pool->m_mutex);

		*(pool->m_pStatus + seq) = THREAD_RUNNING;

		pthread_testcancel();
		(*fun)(argument);
		pthread_testcancel();
	}

	pthread_cleanup_pop(0);

}

