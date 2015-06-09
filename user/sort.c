#include<inc/lib.h>

#define n 16384

unsigned rand()
{
	static unsigned start = 0;
	start = start * 131 + 13;
	return start;
}

int a[n],b[n];

#define swap(a,b) do { int t=(a); (a)=(b); (b)=(t); } while(0)

void sort(void*para)
{
	int L=*(int*)para;
	int R=*(int*)(para+4);
	if (L+1>=R)
		return;
	if (L+2==R)
	{
		if (a[L]>a[L+1])
			swap(a[L],a[L+1]);
		return;
	}
	int t=(L+R)/2;
	int stal[2],star[2];
	stal[0]=L;
	stal[1]=t;
	star[0]=t;
	star[1]=R;
	if (R-L>16)
	{
		int r=create_thread(sort,(void*)stal);
		sort(star);
		wait_thread(r);
	} else
	{
		sort(stal);
		sort(star);
	}

	int i,j,c;
	for(i=L,j=t,c=L;i<t||j<R;)
	{
		if (j<R&&(i==t||a[i]>a[j]))
			b[c++]=a[j++];
		else
			b[c++]=a[i++];
	}
	for(i=L;i<R;i++)
		a[i]=b[i];
}

void umain(int argc, char **argv)
{
	int i;
	for(i=n-1;i>=0;i--)
		a[i]=rand();
	/*
	for(i=0;i<n;i++)
		cprintf("%d ",a[i]);
	cprintf("\n");
	*/
	uint64_t start_time = read_tsc(), end_time;
	int sta[2];
	sta[0]=0;
	sta[1]=n;
	//end_time = read_tsc();
	sort((void*)sta);
	end_time = read_tsc();
	uint64_t delta = end_time - start_time;
	cprintf("Cost time: %d%09d\n", (int)(delta / 1000000000), (int)(delta % 1000000000));
	/*
	for(i=0;i<n;i++)
		cprintf("%d ",a[i]);
	cprintf("\n");
	*/
}

