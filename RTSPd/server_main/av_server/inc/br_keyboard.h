#ifndef BR_KEYBOARD_H
#define BR_KEYBOARD_H

#ifdef  __cplusplus
extern "C" {
#endif

#define DEBUG 1
#define RELEASE 0

typedef void (*callback_key_fun)(int channel);

//该库的版本号，便于跟踪 ＢＵＧ,当前为2
int BRLib_Keyboard_Version();

//Lib初始化
//debug_or_release为测试参数
void BRLib_Keyboard_Init(int debug_or_release);

//Lib退出
void BRLib_Keyboard_Exit();


void BRLib_Keyboard_Start(callback_key_fun f);





#ifdef  __cplusplus
}
#endif

#endif
