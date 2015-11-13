/*author  lishiyuan  on 2015-08-07*/
#ifndef __OWL_CHECK_IDR
#define __OWL_CHECK_IDR
// return 0: is IDR, -1: no IDR
int OWL_check_h264_IDR(unsigned char *buf, int len);
int OWL_check_mpeg2_IDR(unsigned char *buf, int len);
#endif