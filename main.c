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

// ��������
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

// ������
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

// �����ʼ��
void init_ui()
{
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 11);
    printf("\n***************************************\n");
    printf("*      ����ϰ����ϵͳ V2.0         *\n");
    printf("***************************************\n\n");
    SetConsoleTextAttribute(hConsole, 7);
}

// ���Ʋ˵�
void draw_menu()
{
    printf("\n��ѡ�������\n");
    printf("1. ��ѯϵͳ���ڰ���\n");
    printf("2. �����Զ�������\n");
    printf("3. �˳�ϵͳ\n");
    printf("������ѡ��(1-3): ");
}

// ��ȡ�û�ѡ��
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
        printf("������Ч�����������룺");
        while (getchar() != '\n')
            ;
    }
}

// ����ѡ��
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
    printf("\n�����������...");
    _getch();
    system("cls");
    init_ui();
}

// ��ȡϵͳ���ڴ���
void get_system_date()
{
    SYSTEMTIME sysDate, now;
    GetLocalTime(&now);
    print_date_info(now);

    if (now.wDayOfWeek <= 3 || now.wDayOfWeek == 0)
    { // ������֮ǰ������Ϊ0��
        sysDate = now;
    }
    else
    {
        // ��1��
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

// �����Զ�����������
void input_custom_date()
{
    char input[DATE_LEN];
    SYSTEMTIME customDate = {0};

    printf("����������(��ʽYYYYMMDD): ");
    while (1)
    {
        scanf_s("%8s", input, (unsigned)_countof(input));
        if (strlen(input) != 8)
        {
            printf("��ʽ�������������룺");
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
        printf("��Ч���ڣ����������룺");
    }
    calculate_schedule(customDate, 1);
}

// ������֤
int validate_date(int y, int m, int d)
{
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (y < 1900 || m < 1 || m > 12 || d < 1)
        return 0;
    if (is_leap_year(y))
        month_days[1] = 29;
    return d <= month_days[m - 1];
}

// �����ж�
int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// �����Ű����
void calculate_schedule(SYSTEMTIME target, int is_custom)
{
    Config config;
    read_config(&config);

    // ����Ŀ���ܵ���һ
    SYSTEMTIME week_start = get_week_start(target);

    // ������������ڻ�׼���ڣ�
    int days = days_between(config.base_date, week_start);
    if (days < 0)
    {
        printf("\n�����������ڻ�׼���ڣ�");
        return;
    }
    int weeks = days / 7;
    char *week_type = (weeks % 2) ? "˫��" : "����";

    // ��ʾ�߼�
    printf("\n\n=== %04d-%02d-%02d �����ܰ��� ===",
           week_start.wYear, week_start.wMonth, week_start.wDay);

    if (!is_custom)
    { // ϵͳ��ѯʱ��ʾ�ܷ�Χ��ʾ
        SYSTEMTIME now;
        GetLocalTime(&now);
        int day_diff = days_between(now, week_start);
        if (day_diff > 0)
        {
            printf(" �����ܰ��ţ�");
        }
        else
        {
            printf(" �����ܰ��ţ�");
        }
    }

    printf("\n�����ͣ�%s", week_type);
    printf("\n------------------------------");

    // ��ӡ�����Ű���Ϣ��ԭ��print_schedule�߼��������ˣ�
    char weekdays[][4] = {"��һ", "�ܶ�", "����", "����", "����", "����", "����"};
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
                printf("\n�� %s %04d-%02d-%02d �� %s",
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
        printf("\n��ʹ��Ĭ�����á�");
        if (strcmp(week_type, "����") == 0)
        {
            SYSTEMTIME tue = calculate_weekday(week_start, 2);
            SYSTEMTIME wed = calculate_weekday(week_start, 3);
            printf("\n�� �ܶ� %04d-%02d-%02d �� 6������ϰ", tue.wYear, tue.wMonth, tue.wDay);
            printf("\n�� ���� %04d-%02d-%02d �� 10������ϰ", wed.wYear, wed.wMonth, wed.wDay);
        }
        else
        {
            SYSTEMTIME wed = calculate_weekday(week_start, 3);
            printf("\n�� ���� %04d-%02d-%02d �� 9������ϰ", wed.wYear, wed.wMonth, wed.wDay);
        }
    }
    printf("\n==============================\n");
}

// ��ȡ�ܿ�ʼ���ڣ���һ��
SYSTEMTIME get_week_start(SYSTEMTIME date)
{
    SYSTEMTIME start = date;
    int current_weekday = date.wDayOfWeek == 0 ? 7 : date.wDayOfWeek; // ת������Ϊ7
    int diff = current_weekday - 1;                                   // ��������һ�Ĳ�ֵ

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

// ����ָ�����ڼ�������
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

// �����ļ�����
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

// ���������
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

// ��ʼ��Ĭ������
void write_default_config()
{
    FILE *fp;
    fopen_s(&fp, CONFIG_FILE, "w");
    fprintf(fp, "20250217\n");
    fprintf(fp, "����|�ܶ�|6��\n");
    fprintf(fp, "����|����|10��\n");
    fprintf(fp, "˫��|����|9��\n");
    fclose(fp);
}

// ��ӡ������Ϣ
void print_date_info(SYSTEMTIME date)
{
    char *weekdays[] = {"��", "һ", "��", "��", "��", "��", "��"};
    printf("\n��ǰ���ڣ�%04d-%02d-%02d ����%s\n",
           date.wYear, date.wMonth, date.wDay,
           weekdays[date.wDayOfWeek]);
}