#include <lpc21xx.h>
#include "type.h"
#include "lcd.h"
#include "kpm.h"
#include "delay.h"
#include "eeprom.h"

#define BAL_ADDR_H  0x0010
#define BAL_ADDR_L  0x0011
#define MIN_BAL     500

void EEPROM_WriteBalance(u16 bal)
{
    Bytewrite_25LC512(BAL_ADDR_H, (bal >> 8) & 0xFF);
    Bytewrite_25LC512(BAL_ADDR_L, bal & 0xFF);
}

u16 EEPROM_ReadBalance(void)
{
    u16 bal;

    bal = ByteRead_25LC512(BAL_ADDR_H);
    bal = (bal << 8) | ByteRead_25LC512(BAL_ADDR_L);

    return bal;
}

void lcd_print_num(u16 num)
{
    char arr[6];
    int i = 0, j;

    if(num == 0)
    {
        char_lcd('0');
        return;
    }

    while(num > 0)
    {
        arr[i++] = (num % 10) + '0';
        num = num / 10;
    }

    for(j = i - 1; j >= 0; j--)
        char_lcd(arr[j]);
}

u16 GetAmount(void)
{
    u16 amt = 0;
    u32 key;

    cmd_lcd(0x01);
    cmd_lcd(0x80);
    str_lcd("Enter Amount");
    cmd_lcd(0xC0);

    while(1)
    {
        key = keyscan();

        if(key >= '0' && key <= '9')
        {
            amt = (amt * 10) + (key - '0');
            char_lcd(key);
        }
        else if(key == 'A')     // A = ENTER
        {
            break;
        }
        else if(key == 'D')     // D = DELETE
        {
            amt = amt / 10;
            cmd_lcd(0x10);      // cursor left
            char_lcd(' ');
            cmd_lcd(0x10);
        }
    }

    return amt;
}

void Balance_Enquiry(void)
{
    u16 bal;

    bal = EEPROM_ReadBalance();

    cmd_lcd(0x01);
    cmd_lcd(0x80);
    str_lcd("Balance:");
    cmd_lcd(0xC0);
    lcd_print_num(bal);

    delay_ms(2000);
}

void Withdraw(void)
{
    u16 bal, amt;

    bal = EEPROM_ReadBalance();
    amt = GetAmount();

    cmd_lcd(0x01);
    cmd_lcd(0x80);

    if(bal >= (amt + MIN_BAL))
    {
        bal = bal - amt;
        EEPROM_WriteBalance(bal);

        str_lcd("Withdraw OK");
        cmd_lcd(0xC0);
        str_lcd("Bal:");
        lcd_print_num(bal);
    }
    else
    {
        str_lcd("Low Balance");
        cmd_lcd(0xC0);
        str_lcd("Min Bal 500");
    }

    delay_ms(2000);
}

void Deposit(void)
{
    u16 bal, amt;

    bal = EEPROM_ReadBalance();
    amt = GetAmount();

    bal = bal + amt;
    EEPROM_WriteBalance(bal);

    cmd_lcd(0x01);
    cmd_lcd(0x80);
    str_lcd("Deposit OK");
    cmd_lcd(0xC0);
    str_lcd("Bal:");
    lcd_print_num(bal);

    delay_ms(2000);
}

void ATM_Menu(void)
{
    u32 key;

    /* Initial balance.
       Run this only once if balance shows garbage.
       After first test, you can comment this line. */
    EEPROM_WriteBalance(5000);

    while(1)
    {
        cmd_lcd(0x01);
        cmd_lcd(0x80);
        str_lcd("1.Bal 2.With");
        cmd_lcd(0xC0);
        str_lcd("3.Dep 4.Exit");

        key = keyscan();

        switch(key)
        {
            case '1':
                Balance_Enquiry();
                break;

            case '2':
                Withdraw();
                break;

            case '3':
                Deposit();
                break;

            case '4':
                cmd_lcd(0x01);
                cmd_lcd(0x80);
                str_lcd("ATM Exit");
                delay_ms(1000);
                return;

            default:
                cmd_lcd(0x01);
                cmd_lcd(0x80);
                str_lcd("Wrong Option");
                delay_ms(1000);
        }
    }
}
