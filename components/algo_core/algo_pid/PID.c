#include "PID.h"

                 // Device header

// PID控制参数和变量
float Err1 = 0, last_err1 = 0, next_err1 = 0; // Err1: 当前误差，last_err1: 上一次误差，next_err1: 下一次误差
float pwm1 = 0;                            // pwm1: PWM输出值
float add1 = 0;                             // add1: PID计算出的增量值
float p1 = 1.2;                             // p1: PID的比例系数
float i1 = 0.1;                             // i1: PID的积分系数
float d1 = 0;                               // d1: PID的微分系数

// 简单绝对值函数
// 功能：计算整数a的绝对值
// 参数：int a - 输入的整数
// 返回值：int16_t - 输入整数的绝对值
int16_t myabs(int a)
{       
    int temp;
    if(a < 0)  temp = -a;  
    else temp = a;
    return temp;
}

// PWM限幅函数
// 功能：限制PWM值在0到99之间
// 参数：无
// 返回值：无
void pwm_control1()
{
    if(pwm1 > 99)
        pwm1 = 99;
    if(pwm1 < 0)
        pwm1 = 0;
}

// PID控制函数
// 功能：根据编码电机的速度和目标速度，计算PWM输出值以控制电机速度
// 参数：int16_t speed1 - 编码电机当前速度，float tar1 - 目标速度
// 返回值：float - 计算后的PWM值
float pid1(int16_t speed1, float tar1)
{
	if (tar1 == 0)
    {
        Err1 = 0;
        last_err1 = 0;
        next_err1 = 0;
        add1 = 0;
        pwm1 = 0;
        return 0;
    }
	
    speed1 = myabs(speed1); // 取绝对值以消除速度的方向
    Err1 = tar1 - speed1;   // 计算速度误差
    add1 = p1 * (Err1 - last_err1) + i1 * Err1 + d1 * (Err1 + next_err1 - 2 * last_err1); // 根据PID公式计算增量
    pwm1 += add1;          // 更新PWM值
    pwm_control1();        // 限制PWM值在合理范围内
    next_err1 = last_err1; // 更新误差值，为下一次迭代做准备
    last_err1 = Err1;
    return pwm1;           // 返回计算后的PWM值
}
