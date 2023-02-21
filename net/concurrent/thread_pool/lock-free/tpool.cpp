/**
 *    author:  mikezheng
 *    created: 01-08-2022 14:24:58
 *    kfifo: https://zhuanlan.zhihu.com/p/500070147
**/

#include <csapp.h>

using namespace std;
using namespace Json; 

#define MAX_THREAD_NUM 512

#define WORK_QUEUE_POWER 16
#define WORK_QUEUE_SIZE (1 << WORK_QUEUE_POWER)
#define WORK_QUEUE_MASK (WORK_QUEUE_SIZE - 1)

#define thread_out_val(thread) (__sync_val_compare_and_swap(&(thread)->out, 0, 0))
#define thread_queue_len(thread) ((thread)->in - thread_out_val(thread))
#define thread_queue_empty(thread) (thread_queue_len(thread) == 0)
#define thread_queue_full(thread) (thread_queue_len(thread) == WORK_QUEUE_SIZE)

// mod and &: https://blog.csdn.net/Xxc6018/article/details/107411126
#define queue_offset(val) ((val) & WORK_QUEUE_MASK)

static int num;
static pthread_t main_tid;
static volatile int global_num_thread = 0;

// typedef unsigned long int pthread_t;

typedef struct tpool_work {
	void* (*routine) (void*);
	void *arg;
	struct tpool_work *next;
} tpool_work_t;

typedef struct {
	pthread_t id; 
	bool shutdown;
	unsigned int in, out;
	tpool_work_t request_q[WORK_QUEUE_SIZE];
} thread_t; // _t: type

// tpool_t use schedule_thread_func, schedule_thread_func use tpool_t.
typedef struct tpool tpool_t;

/*
 * func pointer:
 * 1. https://blog.csdn.net/wu_junwu/article/details/109067004
 * 2. http://c.biancheng.net/view/228.html
 */
typedef thread_t* (*schd_thread_func) (tpool_t* tpool);

enum schedule_type {
  ROUND_ROBIN, LEAST_LOAD
} schd_algorithm;

// use schd_func distribute thread to work queue
struct tpool {
  int num;
  thread_t threads[MAX_THREAD_NUM];
	schd_thread_func schd_func;
};

static int tpool_empty(tpool_t* tpool) {
	for (int i = 0 ; i < tpool->num ; i ++ ) {
		if (!thread_queue_empty(&tpool->threads[i])) return 0; // no empty
	}
	return 1; // empty
}

static thread_t* round_robin_schd(tpool_t* tpool) {
  static int cur_thread_index = -1;
  cur_thread_index = (cur_thread_index + 1) % tpool->num; // 轮询
  return &tpool->threads[cur_thread_index];
}

// 正在工作数最少的线程分配工作
static thread_t* least_load_schd(tpool_t* tpool) {
	int min_num_works_index = 0;
	for (int i = 1 ; i < tpool->num ; i ++ ) {
		if (thread_queue_len(&tpool->threads[i]) < thread_queue_len(&tpool->threads[min_num_works_index])) min_num_works_index = i;
	}
	return &tpool->threads[min_num_works_index];
}

// array
static const schd_thread_func schd_algorithms[] = {
  [ROUND_ROBIN] = round_robin_schd,
  [LEAST_LOAD] = least_load_schd
};

static void sig_do_nothing(int signo) {
	return ;
}

// 原子操作实现main-son threads并发, thread->in, thread->out, thread is shared source.
static tpool_work_t* get_work_concurrently(thread_t* thread) {
	tpool_work_t* work = NULL; unsigned int tmp;
	// work out and do, queue size - 1. So out + 1.
	// if failed, try again. else break.
	do {
		work = NULL;
		if (thread_queue_len(thread) <= 0) break;
		tmp = thread->out;
		// get work with index out
		work = &thread->request_q[queue_offset(tmp)];
	} while (!__sync_bool_compare_and_swap(&thread->out, tmp, tmp+1));
	return work;
}

// control signal of thread
// without work func
static void* tpool_thread(void* arg) {
	int sig_catch_no;
	thread_t* thread = (thread_t*)arg;
	tpool_work_t *work = NULL;

	__sync_fetch_and_add(&global_num_thread, 1);
	pthread_kill(main_tid, SIGUSR1); // created

	sigset_t signal_mask, oldmask;
	sigemptyset(&oldmask); sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGUSR1);

	while (true) {
		if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
			cerr << "sig_block error!" << endl; pthread_exit(NULL);
		}

		while (thread_queue_empty(thread) && !thread->shutdown) {
			cout << "I'm sleep" << endl;
			if (sigwait(&signal_mask, &sig_catch_no)) {
				cerr << "sig_wait error!" << endl; pthread_exit(NULL);
			}
		}

		if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL)) {
			cerr << "sig_setmask error!" << endl; pthread_exit(NULL);
		}
		cout << "I'm awake" << endl;

		if (thread->shutdown) {
			cout << "exited" << endl; pthread_exit(NULL);
		}

		work = get_work_concurrently(thread);
		if (work) { // 执行请求响应
			(*(work->routine))(work->arg);
		}
		if (thread_queue_empty(thread)) pthread_kill(main_tid, SIGUSR1); // thread work finished
	}
}

static void init_thread(tpool_t* tpool, int idx) {
	if (pthread_create(&tpool->threads[idx].id, NULL, tpool_thread, &tpool->threads[idx])) {
		cerr << "pthread_create error!" << endl; exit(0);
	}
}

static int wait_for_thread_registed() {
	int sig_catch_no;
	sigset_t signal_mask, oldmask; // signal set
	sigemptyset(&signal_mask); sigemptyset(&oldmask);

	// 开启信号屏蔽（create 信号来了，阻塞信号）
	sigaddset(&signal_mask, SIGUSR1);
	if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
		cerr << "sig_block error!" << endl; return -1;
	}

	// 信号出未决队列，保持阻塞状态，一旦解除阻塞，进行信号处理函数
	while (global_num_thread < num) {
		if (sigwait(&signal_mask, &sig_catch_no)) {
			cerr << "sigwait error!" << endl; return -1;
		}
	}

  // 解除信号屏蔽，处理信号
	if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL)) {
		cerr << "sig_setmask error!" << endl; return -1;
	}
	return 0;
}

static bool parseJson(string path) {
  ifstream ifs(path);
  Reader rd; Value root;
  rd.parse(ifs, root);
  if (root.isObject()) {
		num = root["num_threads"].asInt();
		schd_algorithm = schedule_type(root["schd_algorithm"].asInt());
    return true;
  }
  return false;
}

void* tpool_init(string path) {
	if (!parseJson(path)) {
		cerr << "parse error" << endl;
		return NULL;
	}

	if (num <= 0) {
		cerr << "Illegal thread num !!!" << endl;
		return NULL;
	} else if (num > MAX_THREAD_NUM) {
		cerr << "Too many threads !!!" << endl;
		return NULL;
	}

	tpool_t* tpool = new tpool_t();
	if (tpool) {
		tpool->num = num;
		tpool->schd_func = schd_algorithms[schd_algorithm];
		// 之前的信号不做处理
		if (signal(SIGUSR1, sig_do_nothing) == SIG_ERR) return NULL;
		main_tid = pthread_self();
		for (int i = 0 ; i < tpool->num ; i ++ ) init_thread(tpool, i);
		if (wait_for_thread_registed() < 0) { // 检查是否创建完num个线程
			cerr << "pending queue error!" << endl; pthread_exit(NULL);
		}
		return (void*)tpool;
	} else {
		cerr << "thread pool create failed!" << endl;
		return NULL;
	}
}

static int dispatch_work(tpool_t* tpool, thread_t* thread, void* (*routine) (void*), void* arg) {
	tpool_work_t *work = NULL;
	if (thread_queue_full(thread)) {
		cerr << "request_q of thread is full!" << endl; return -1;
	}
	work = &thread->request_q[queue_offset(thread->in)];
	work->routine = routine; work->arg = arg; work->next = NULL;
	thread->in++;
	if (thread_queue_len(thread) == 1) {
		pthread_kill(thread->id, SIGUSR1); // have work awake thread
	}
	return 0;
}

// destroy
static int migrate_thread_work(tpool_t* tpool, thread_t* from) {
	unsigned int i;
	tpool_work_t* work;
	thread_t* to;

	for (i = from->out ; i < from->in ; i ++ ) {
		work = &from->request_q[queue_offset(i)];
		to = tpool->schd_func(tpool);
		if (dispatch_work(tpool, to, work->routine, work->arg) < 0) return -1;
	}
	return 0;
}

static int isnegtive(int val) {
    return val < 0;
}

static int ispositive(int val) {
    return val > 0;
}

static int get_first_id(int arr[], int len, int (*fun)(int)) {
  for (int i = 0; i < len; i ++ )
    if (fun(arr[i])) return i;
  return -1;
}

// low b func
static void balance_thread_load(tpool_t* tpool) {
  thread_t *from, *to;
  int out, sum = 0, avg, migrate_num, cnt[MAX_THREAD_NUM];
	int first_neg_id, first_pos_id;
	tpool_work_t* work;

  for (int i = 0 ; i < tpool->num ; i ++ ) {
    cnt[i] = thread_queue_len(&tpool->threads[i]);
    sum += cnt[i];
  }

  avg = sum / tpool->num; // avg < real avg
  if (!avg) return ;

  for (int i = 0 ; i < tpool->num ; i ++ ) cnt[i] -= avg;
  while (true) {
		// 迁入
    first_neg_id = get_first_id(cnt, tpool->num, isnegtive);
		// 迁出
    first_pos_id = get_first_id(cnt, tpool->num, ispositive);
		// 样本各观测值与平均数之差的和为零，所以当找不到了neg，那么pos也必定找不到
    if (first_neg_id < 0) break;
    int tmp = cnt[first_neg_id] + cnt[first_pos_id];
    if (tmp > 0) {
			// 取较小值作为迁移数
			migrate_num = -cnt[first_neg_id];
      cnt[first_neg_id] = 0; cnt[first_pos_id] = tmp;
    } else {
      migrate_num = cnt[first_pos_id];
      cnt[first_pos_id] = 0; cnt[first_neg_id] = tmp;
    }

    from = &tpool->threads[first_pos_id];
    to = &tpool->threads[first_neg_id];
    for (int i = 0 ; i < migrate_num ; i ++ ) {
			work = get_work_concurrently(from);
			if (work) {
				dispatch_work(tpool, to, work->routine, work->arg);
			}
    }
  }
	// 因为均值不准确，neg偏小，pos偏大
	// so we let cnt_i = 1
	from = &tpool->threads[first_pos_id];
	/* Just migrate count[first_pos_id] - 1 works to other threads*/
  for (int i = 1; i < cnt[first_pos_id]; i++) {
    to = &tpool->threads[i - 1];
    if (to == from) continue;
    work = get_work_concurrently(from);
    if (work) {
      dispatch_work(tpool, to, work->routine, work->arg);
    }
  }	
}

int tpool_inc_threads(void* pool, int num_inc) {
	tpool_t* tpool = (tpool_t*)pool;

	num = tpool->num + num_inc;
	if (num > MAX_THREAD_NUM) {
		cerr << "Add too many threads!" << endl;
		return -1;
	}

	for (int i = tpool->num ; i < num ; i ++ ) init_thread(tpool, i);

	if (wait_for_thread_registed() < 0) {
		cerr << "create thread error!" << endl; pthread_exit(NULL);
	}
	tpool->num = num;
	balance_thread_load(tpool);
	return 0;
}

void tpool_dec_threads(void* pool, int num_dec) {
	tpool_t* tpool = (tpool_t*)pool;

	if (num_dec > tpool->num) num_dec = tpool->num;
	tpool->num -= num_dec;
	for (int i = tpool->num ; i < num ; i ++ ) {
		tpool->threads[i].shutdown = 1;
		pthread_kill(tpool->threads[i].id, SIGUSR1); // close thread
	}
	for (int i = tpool->num ; i < num ; i ++ ) {
		pthread_join(tpool->threads[i].id, NULL);
		if (migrate_thread_work(tpool, &tpool->threads[i]) < 0)
			cerr << "work lost during migration!" << endl;
	}
	if (tpool->num == 0 && !tpool_empty(tpool)) {
		cerr << "No thread in pool with work unfinished!" << endl;
	}
}

int tpool_add_work(void* pool, void* (*routine) (void*), void* arg) {
	tpool_t* tpool = (tpool_t*)pool;
	thread_t* thread = tpool->schd_func(tpool);
	return dispatch_work(tpool, thread, routine, arg);
}

void tpool_destroy(void* pool, int finish) {
	tpool_t* tpool = (tpool_t*)pool;
	// wait all threads work done
	if (finish) {
		int sig_catch_no;
		sigset_t signal_mask, oldmask;
    sigemptyset (&oldmask); sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGUSR1);
		if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
			cerr << "sig_block error!" << endl;
			pthread_exit(NULL);
		}
		
		while (!tpool_empty(tpool)) {
			if (sigwait(&signal_mask, &sig_catch_no)) {
				cerr << "sigwait error!" << endl;
				pthread_exit(NULL);
			}
		}

		if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL)) {
			cerr << "sig_setmask error!" << endl;
			pthread_exit(NULL);
		}
	}
	// shutdown all threads immediately
	for (int i = 0 ; i < tpool->num ; i ++ ) {
		tpool->threads[i].shutdown = 1;
		pthread_kill(tpool->threads[i].id, SIGUSR1); // close thread
	}
	for (int i = 0 ; i < tpool->num ; i ++ ) {
		pthread_join(tpool->threads[i].id, NULL);
	}
	free(tpool);
}
