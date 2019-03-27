/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */


/*
 * this program stats the various digits combination of Chinese FUCAI 3D ^_^
 */

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "debug.h"

#define    TOTAL_HEZHI_NR     (28)

#define    SINGLE_POS_DGT_NR  (10)

typedef struct {
	int zusan_cnt;
	int zuliu_cnt;
	int hezhi_cnt[TOTAL_HEZHI_NR];
} t_cmb_stat;

static t_cmb_stat gt_cmb_stat = {
	0,
};

static int same_pair_cnt(int i, int j, int k)
{
	int ret = 0;

	if (i == j) ret++;
	if (i == k) ret++;
	if (j == k) ret++;

	return ret;
}

static int cmb_is_zusan(int i, int j, int k)
{
	return same_pair_cnt(i, j, k) == 1;
}

static int cmb_is_zuliu(int i, int j, int k)
{
	return same_pair_cnt(i, j, k) == 0;
}

static int hezhi_of_cmb(int i, int j, int k)
{
	return i+j+k;
}

static void parse_single_cmb(int i, int j, int k)
{
	if (cmb_is_zusan(i, j, k)) {
		gt_cmb_stat.zusan_cnt++;
	}

	if (cmb_is_zuliu(i, j, k)) {
		gt_cmb_stat.zuliu_cnt++;
	}

	gt_cmb_stat.hezhi_cnt[hezhi_of_cmb(i, j, k)]++;
}

static void collect_stat(void)
{
	int i, j, k;

	for (i=0; i<SINGLE_POS_DGT_NR; i++) {
		for (j=0; j<SINGLE_POS_DGT_NR; j++) {
			for (k=0; k<SINGLE_POS_DGT_NR; k++) {
				parse_single_cmb(i, j, k);

			}

		}
	}

}

static void print_cmb_stat(void)
{
	int i;

	printf("zusan cnt: %d\n", gt_cmb_stat.zusan_cnt);
	printf("zuliu cnt: %d\n", gt_cmb_stat.zuliu_cnt);

	printf("hezhi stat:\n");

	for (i=0; i<TOTAL_HEZHI_NR; i++) {
		printf("%-2d : %d\n"
			, i
			, gt_cmb_stat.hezhi_cnt[i]);
	}
}


//test code begin => {
static void test_same_pair_cnt(void)
{
	assert(same_pair_cnt(1,2,3)==0);
	assert(same_pair_cnt(2,3,4)==0);
	assert(same_pair_cnt(1,2,2)==1);
	assert(same_pair_cnt(9,9,9)==3);
}

static void test_cmb_is_zusan(void)
{
	assert(cmb_is_zusan(1,2,3)==0);
	assert(cmb_is_zusan(2,3,4)==0);
	assert(cmb_is_zusan(1,2,2)==1);
	assert(cmb_is_zusan(9,9,9)==0);
}

static void test_cmb_is_zuliu(void)
{
	assert(cmb_is_zuliu(1,2,3)==1);
	assert(cmb_is_zuliu(2,3,4)==1);
	assert(cmb_is_zuliu(1,2,2)==0);
	assert(cmb_is_zuliu(9,9,9)==0);
}

static void test_hezhi_of_cmb(void)
{
	assert(hezhi_of_cmb(1,2,3)==6);
	assert(hezhi_of_cmb(2,3,4)==9);
	assert(hezhi_of_cmb(1,2,2)==5);
	assert(hezhi_of_cmb(9,9,9)==27);
}

static void test_all(void)
{
	test_same_pair_cnt();
	test_cmb_is_zusan();
	test_cmb_is_zuliu();
	test_hezhi_of_cmb();
}
// } <= test code end


int main(int argc, char *argv[])
{
	//test_all();

	collect_stat();
	print_cmb_stat();

	return 0;
}

