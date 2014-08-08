#ifndef __THREADPOOL_H__SDDSFGGAB9980
#define __THREADPOOL_H__SDDSFGGAB9980

#define IF_NULL(p, e)   \
		if (NULL == (p)) { \
			return (e);\
		}      
	

enum 
{
	SUCCESS = 0,
	MALLOC_FAIL = -1,
	PTHREAD_CREATE_FAIL = -2,
	PTHREAD_POOL_NULL = -3,
	TASK_FUNCTION_NULL = -4
};


enum 
{
	THREAD_INIT = 0,
	THREAD_WAITTING,
	THREAD_RUNNING,
	THREAD_DIE	
};


typedef void (*Func)(void *arg);

typedef struct task_element_t TaskElement;
struct task_element_t  
{
	Func         m_pFunc; /* task function */
	void         *m_pArg; /* task function argument */
	TaskElement  *next;
};




typedef struct pthreadpool_t PthreadPool;

struct pthreadpool_t 
{
	pthread_t            *m_pth; /*  pthread array */	
	pthread_mutex_t      m_mutex; /* pthread mutex */
	pthread_cond_t       m_cond;  /* pthread condition */
	int                  *m_pStatus; /* pthread status */

	TaskElement          *m_pTaskHead; /* task queue list head*/
	TaskElement          *m_pTaskTail; /* task queue list tail */
	
	int                   m_iTaskNum; /* task queue num */

	int                   m_iPthreadNum; /* pthread  num */	

};

typedef struct 
{
	PthreadPool *m_pool;
	int          m_iThread;
} PthreadStatus; 


PthreadPool* PthreadPoolInit(int pthreadNums);
int pthreadPoolDestroy(PthreadPool *pool);
int PthreadPoolAddTask(PthreadPool *pool, Func fun, void *arg);
void  *doPthreadPool(void *arg);


#endif
