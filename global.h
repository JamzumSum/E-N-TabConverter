/*
	global.h 仿佛整个算法的神经系统，它使得各个measure的识别不再相互独立，后面的可以借鉴前面的“经验”
	实际上是一种显式的信息传递。
*/
/*
	global.h 中的变量遵循一种 newValue = (oldValue + data) / 2 的更新方法，也就是说，前面的值权重更大，后面的值权重更小。这实际上
	模拟了人“先入为主”的特性。
*/

extern int col;						//乐谱宽度
extern int lineThickness;			//线粗细
extern int rowLenth;				//六线宽度
extern int colLenth;				//小节宽度
extern int valueSignalLen;
extern int characterWidth;			//字符宽度