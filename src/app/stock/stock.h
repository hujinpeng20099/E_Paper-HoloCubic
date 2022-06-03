#ifndef __STOCK_H
#define __STOCK_H

struct StockAppRunData
{
    unsigned int stock_code;
    unsigned int open_price;
    unsigned int cur_price;
    unsigned int end_price;
    unsigned int turnover;
    unsigned long trading_volume;
    unsigned char flag;
    unsigned char up_date; 
};


extern StockAppRunData *stock_run_data;

void stock_init(void);
void update_stock(void);
void stock_gui_init(void);
void display_stock(void);

#endif