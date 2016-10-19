#ifndef _W_TIME_CONST_H_
#define _W_TIME_CONST_H_

namespace wang {   

	// ʱ�䳣��
	enum ETimeConstant
	{
		ETC_Minute					= 60,					// ����, ��λ1s
		ETC_QuarterHour				= 15 * ETC_Minute,		// һ����, ��λ1s
		ETC_HalfHour				= 30 * ETC_Minute,		// ��Сʱ, ��λ1s
		ETC_Hour					= 60 * ETC_Minute,		// Сʱ, ��λ1s
		ETC_Day						= 24 * ETC_Hour,		// ��, ��λ1s
		ETC_Week					= 7 * ETC_Day,			// ��, ��λs
	};

	static const int ASCII_DATETIME_LEN = 20; //2016-04-24 14:06:59
	static const int BASE_TIME = 946598400; //1900 -> 2000-1-1 ������

} // namespace wang

#endif // _W_TIME_CONST_H_