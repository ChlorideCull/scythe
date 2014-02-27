#include "Global.h"
#include "CPUMiner.h"
#include "Util.h"
#include "pthread.h"
#include "RSHash.h"

extern pthread_mutex_t current_work_mutex;
extern Work current_work;

void* Reap_CPU(void* param)
{
	Reap_CPU_param* state = (Reap_CPU_param*)param;

	Work tempwork;
	tempwork.time = 13371337;

	uchar tempdata[544];
	memset(tempdata, 0, 512);

	while(!shutdown_now)
	{
		if (current_work.old)
		{
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time)
		{
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			*(uint*)&tempdata[104] = state->thread_id;
		}
		BlockHash_1(tempdata, tempdata+512);
		if (tempdata[542] == 0 && tempdata[543] == 0)
		{
			bool below=true;
			for(int i=31; i>=0; --i)
			{
				if (tempdata[512+i] > tempwork.target_share[31-i])
				{
					below=false;
					break;
				}
				if (tempdata[512+i] < tempwork.target_share[31-i])
				{
					break;
				}
			}
			if (below)
			{
				vector<uchar> share(tempdata, tempdata+128);
				pthread_mutex_lock(&state->share_mutex);
				state->shares_available = true;
				state->shares.push_back(share);
				pthread_mutex_unlock(&state->share_mutex);
			}
		}
		++state->hashes;
		++*(uint*)&tempdata[100];
	}
	pthread_exit(NULL);
	return NULL;
}

vector<Reap_CPU_param> CPUstates;

void CPUMiner::Init()
{
	if (globalconfs.cputhreads == 0)
	{
#ifdef CPU_MINING_ONLY
		cout << "Config warning: cpu_mining_threads 0" << endl;
#endif
		return;
	}
	for(uint i=0; i<globalconfs.cputhreads; ++i)
	{
		Reap_CPU_param state;
		pthread_mutex_t initializer = PTHREAD_MUTEX_INITIALIZER;

		state.share_mutex = initializer;
		state.shares_available = false;

		state.hashes = 0;

		state.thread_id = i|0x80000000;

		CPUstates.push_back(state);
	}

	cout << "Creating " << CPUstates.size() << " CPU thread" << (CPUstates.size()==1?"":"s") << "." << endl;
	for(uint i=0; i<CPUstates.size(); ++i)
	{
		cout << i+1 << "...";
		pthread_attr_t attr;
	    pthread_attr_init(&attr);
		int schedpolicy;
		pthread_attr_getschedpolicy(&attr, &schedpolicy);
		int schedmin = sched_get_priority_min(schedpolicy);
		int schedmax = sched_get_priority_max(schedpolicy);
		if (i==0 && schedmin == schedmax)
		{
			cout << "Warning: can't set thread priority" << endl;
		}
		sched_param schedp;
		schedp.sched_priority = schedmin;
		pthread_attr_setschedparam(&attr, &schedp);

		pthread_create(&CPUstates[i].thread, &attr, Reap_CPU, (void*)&CPUstates[i]);
		pthread_attr_destroy(&attr);
	}
	cout << "done" << endl;
}

void CPUMiner::Quit()
{
}