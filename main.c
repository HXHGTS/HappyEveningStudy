#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define CONFIG_FILE "config.ini"
#define MAX_CONFIG 10
#define DATE_LEN 9
#define LINE_LEN 50

typedef struct
{
    char week_type[10];
    char weekday[10];
    char class_info[20];
} Schedule;

typedef struct
{
    SYSTEMTIME base_date;
    Schedule schedules[MAX_CONFIG];
    int count;
} Config;

// 函数声明
void init_ui();
void draw_menu();
int get_choice();
void process_option(int option);
void get_system_date();
void input_custom_date();
int validate_date(int y, int m, int d);
int is_leap_year(int year);
void calculate_schedule(SYSTEMTIME target, int is_custom);
void read_config(Config *config);
void write_default_config();
void save_config(Config *config);
int days_between(SYSTEMTIME start, SYSTEMTIME end);
SYSTEMTIME get_week_start(SYSTEMTIME date);
SYSTEMTIME calculate_weekday(SYSTEMTIME start, int weekday);
void print_date_info(SYSTEMTIME date);

// 主函数
int main()
{
    init_ui();
    while (1)
    {
        draw_menu();
        int choice = get_choice();
        if (choice == 3)
            break;
        process_option(choice);
    }
    return 0;
}

// 界面初始化
void init_ui()
{
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 11);
    printf("\n***************************************\n");
    printf("*      晚自习管理系统 V2.0         *\n");
    printf("***************************************\n\n");
    SetConsoleTextAttribute(hConsole, 7);
}

// 绘制菜单
void draw_menu()
{
    printf("\n请选择操作：\n");
    printf("1. 查询系统日期安排\n");
    printf("2. 输入自定义日期\n");
    printf("3. 退出系统\n");
    printf("请输入选项(1-3): ");
}

// 获取用户选择
int get_choice()
{
    int c;
    while (1)
    {
        if (scanf_s("%d", &c) == 1)
        {
            if (c >= 1 && c <= 3)
                return c;
        }
        printf("输入无效，请重新输入：");
        while (getchar() != '\n')
            ;
    }
}

// 处理选项
void process_option(int option)
{
    system("cls");
    switch (option)
    {
    case 1:
        get_system_date();
        break;
    case 2:
        input_custom_date();
        break;
    }
    printf("\n按任意键继续...");
    _getch();
    system("cls");
    init_ui();
}

// 获取系统日期处理
void get_system_date()
{
    SYSTEMTIME sysDate, now;
    GetLocalTime(&now);
    print_date_info(now);

    if (now.wDayOfWeek <= 3 || now.wDayOfWeek == 0)
    { // 周三及之前（周日为0）
        sysDate = now;
    }
    else
    {
        // 加1周
        FILETIME ft;
        ULARGE_INTEGER uli;
        SystemTimeToFileTime(&now, &ft);
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uli.QuadPart += 7 * 24 * 3600 * 10000000ULL;
        ft.dwLowDateTime = uli.LowPart;
        ft.dwHighDateTime = uli.HighPart;
        FileTimeToSystemTime(&ft, &sysDate);
    }
    calculate_schedule(sysDate, 0);
}

// 处理自定义日期输入
void input_custom_date()
{
    char input[DATE_LEN];
    SYSTEMTIME customDate = {0};

    printf("请输入日期(格式YYYYMMDD): ");
    while (1)
    {
        scanf_s("%8s", input, (unsigned)_countof(input));
        if (strlen(input) != 8)
        {
            printf("格式错误，请重新输入：");
            continue;
        }

        char year[5] = {0};
        char month[3] = {0};
        char day[3] = {0};
        strncpy_s(year, 5, input, 4);
        strncpy_s(month, 3, input + 4, 2);
        strncpy_s(day, 3, input + 6, 2);

        customDate.wYear = atoi(year);
        customDate.wMonth = atoi(month);
        customDate.wDay = atoi(day);

        if (validate_date(customDate.wYear, customDate.wMonth, customDate.wDay))
        {
            break;
        }
        printf("无效日期，请重新输入：");
    }
    calculate_schedule(customDate, 1);
}

// 日期验证
int validate_date(int y, int m, int d)
{
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (y < 1900 || m < 1 || m > 12 || d < 1)
        return 0;
    if (is_leap_year(y))
        month_days[1] = 29;
    return d <= month_days[m - 1];
}

// 闰年判断
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// 核心排班计算
void calculate_schedule(SYSTEMTIME target, int is_custom)
{
    Config config;
    read_config(&config);

    // 计算目标周的周一
    SYSTEMTIME week_start = get_week_start(target);

    // 计算周数差（基于基准日期）
    int days = days_between(config.base_date, week_start);
    if (days < 0)
    {
        printf("\n错误：日期早于基准日期！");
        return;
    }
    int weeks = days / 7;
    char *week_type = (weeks % 2) ? "双周" : "单周";

    // 显示逻辑
    printf("\n\n=== %04d-%02d-%02d 所在周安排 ===",
           week_start.wYear, week_start.wMonth, week_start.wDay);

    if (!is_custom)
    { // 系统查询时显示周范围提示
        SYSTEMTIME now;
        GetLocalTime(&now);
        int day_diff = days_between(now, week_start);
        if (day_diff > 0)
        {
            printf(" （下周安排）");
        }
        else
        {
            printf(" （本周安排）");
        }
    }

    printf("\n周类型：%s", week_type);
    printf("\n------------------------------");

    // 打印具体排班信息（原有print_schedule逻辑整合至此）
    char weekdays[][4] = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    int found = 0;

    for (int i = 0; i < config.count; i++)
    {
        if (strcmp(config.schedules[i].week_type, week_type) != 0)
            continue;

        for (int w = 0; w < 7; w++)
        {
            if (strcmp(config.schedules[i].weekday, weekdays[w]) == 0)
            {
                SYSTEMTIME class_date = calculate_weekday(week_start, w + 1);
                printf("\n· %s %04d-%02d-%02d → %s",
                       weekdays[w],
                       class_date.wYear,
                       class_date.wMonth,
                       class_date.wDay,
                       config.schedules[i].class_info);
                found = 1;
                break;
            }
        }
    }

    if (!found)
    {
        printf("\n【使用默认配置】");
        if (strcmp(week_type, "单周") == 0)
        {
            SYSTEMTIME tue = calculate_weekday(week_start, 2);
            SYSTEMTIME wed = calculate_weekday(week_start, 3);
            printf("\n· 周二 %04d-%02d-%02d → 6班晚自习", tue.wYear, tue.wMonth, tue.wDay);
            printf("\n· 周三 %04d-%02d-%02d → 10班晚自习", wed.wYear, wed.wMonth, wed.wDay);
        }
        else
        {
            SYSTEMTIME wed = calculate_weekday(week_start, 3);
            printf("\n· 周三 %04d-%02d-%02d → 9班晚自习", wed.wYear, wed.wMonth, wed.wDay);
        }
    }
    printf("\n==============================\n");
}

// 获取周开始日期（周一）
SYSTEMTIME get_week_start(SYSTEMTIME date)
{
    SYSTEMTIME start = date;
    int current_weekday = date.wDayOfWeek == 0 ? 7 : date.wDayOfWeek; // 转换周日为7
    int diff = current_weekday - 1;                                   // 计算与周一的差值

    FILETIME ft;
    ULARGE_INTEGER uli;
    SystemTimeToFileTime(&date, &ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uli.QuadPart -= diff * 24 * 3600 * 10000000ULL;
    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;
    FileTimeToSystemTime(&ft, &start);

    return start;
}

// 计算指定星期几的日期
SYSTEMTIME calculate_weekday(SYSTEMTIME start, int weekday)
{
    SYSTEMTIME result = start;
    int diff = (weekday - 1) % 7;

    FILETIME ft;
    ULARGE_INTEGER uli;
    SystemTimeToFileTime(&start, &ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uli.QuadPart += diff * 24 * 3600 * 10000000ULL;
    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;
    FileTimeToSystemTime(&ft, &result);

    return result;
}

// 配置文件处理
void read_config(Config *config)
{
    FILE *fp;
    if (fopen_s(&fp, CONFIG_FILE, "r") != 0)
    {
        write_default_config();
        fopen_s(&fp, CONFIG_FILE, "r");
    }

    char line[LINE_LEN];
    fgets(line, LINE_LEN, fp);
    sscanf_s(line, "%4hd%2hd%2hd",
             &config->base_date.wYear,
             &config->base_date.wMonth,
             &config->base_date.wDay);

    config->count = 0;
    while (fgets(line, LINE_LEN, fp) && config->count < MAX_CONFIG)
    {
        if (sscanf_s(line, "%[^|]|%[^|]|%[^\n]",
                     config->schedules[config->count].week_type, 10,
                     config->schedules[config->count].weekday, 10,
                     config->schedules[config->count].class_info, 20) == 3)
        {
            config->count++;
        }
    }
    fclose(fp);
}

// 天数差计算
int days_between(SYSTEMTIME start, SYSTEMTIME end)
{
    FILETIME ftStart, ftEnd;
    SystemTimeToFileTime(&start, &ftStart);
    SystemTimeToFileTime(&end, &ftEnd);

    ULARGE_INTEGER uStart, uEnd;
    uStart.LowPart = ftStart.dwLowDateTime;
    uStart.HighPart = ftStart.dwHighDateTime;
    uEnd.LowPart = ftEnd.dwLowDateTime;
    uEnd.HighPart = ftEnd.dwHighDateTime;

    LONGLONG diff = uEnd.QuadPart - uStart.QuadPart;
    return (int)(diff / (10000000LL * 3600 * 24));
}

// 初始化默认配置
void write_default_config()
{
    FILE *fp;
    fopen_s(&fp, CONFIG_FILE, "w");
    fprintf(fp, "20250217\n");
    fprintf(fp, "单周|周二|6班\n");
    fprintf(fp, "单周|周三|10班\n");
    fprintf(fp, "双周|周三|9班\n");
    fclose(fp);
}

// 打印日期信息
void print_date_info(SYSTEMTIME date)
{
    char *weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    printf("\n当前日期：%04d-%02d-%02d 星期%s\n",
           date.wYear, date.wMonth, date.wDay,
           weekdays[date.wDayOfWeek]);
}