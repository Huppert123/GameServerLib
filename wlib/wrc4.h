#ifndef _W_RC4_H_
#define _W_RC4_H_

namespace wang
{
    class wRC4 
    {
	public:
        wRC4();

        void set_key(int len, const unsigned char* data);
        
        void process(int len, const unsigned char* in_data, unsigned char* out_data);
		
    private:    
        unsigned char m_data[256];
        int m_x;
        int m_y;
	};
}

#endif //_W_RC4_H_