#ifndef __BILIBILI_H
#define __BILIBILI_H


struct BilibiliAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

void bilibili_init(void);
void update_fans_num(void);

#ifdef __cplusplus
extern "C"
{
#endif

    extern struct BilibiliAppRunData *bilibili_run_data;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

