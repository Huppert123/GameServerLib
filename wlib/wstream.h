#ifndef _W_STREAM_H_
#define _W_STREAM_H_

#include <string>

namespace wang {
	
	class wbuffer;

	class wstream
	{
	public:
		wstream();
		wstream(wbuffer* buf_ptr);
		~wstream();

	public:
		void init(wbuffer* buf_ptr);

	public:
		const wbuffer* get_buf() const { return m_buf_ptr; }
		void append(const char * p, int len);

	public:
		wstream& operator<<(bool);
		wstream& operator<<(char);

		wstream& operator<<(signed char);
		wstream& operator<<(unsigned char);

		wstream& operator<<(signed short);
		wstream& operator<<(unsigned short);

		wstream& operator<<(signed int);
		wstream& operator<<(unsigned int);

		wstream& operator<<(signed long);
		wstream& operator<<(unsigned long);

		wstream& operator<<(signed long long);
		wstream& operator<<(unsigned long long);

		wstream& operator<<(const char *);
		wstream& operator<<(const std::string&);

		wstream& operator<<(float);
		wstream& operator<<(double);

	private:
		wstream(const wstream& rht);
		wstream& operator=(const wstream& rht);

	private:
		wbuffer* m_buf_ptr;
	};

}
#endif // _W_STREAM_H_
