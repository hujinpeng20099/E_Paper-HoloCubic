#include "app/stock/stock.h"

//key = your api key code = stock number
#define STOCK_API    "http://api.tianapi.com/finance/index?key=72f5b2592703a78caf052d420ca68127&code=sh000001&list=0"


struct StockAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

void stock_init(void)
{

}



