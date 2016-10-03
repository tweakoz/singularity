#include "krzio.h"

std::string formatString( const char* formatstring, ... )
{
        std::string rval;
        char formatbuffer[512];

        va_list args;
        va_start(args, formatstring);
        //buffer.vformat(formatstring, args);
        vsnprintf( &formatbuffer[0], sizeof(formatbuffer), formatstring, args );
        va_end(args);
        rval = formatbuffer;
        return rval;
}

void SplitString( const std::string& instr, std::vector<std::string>& splitvect, const char* pdelim )
{
	if( instr.length() )
	{
		const char* psrc = instr.c_str();

		size_t istrlen = instr.length();

		char* buffer = new char[ istrlen+1024 ];
		memset( buffer, 0, istrlen+1024 );
		strcpy( buffer, instr.c_str() );
		char *tok = strtok( buffer, pdelim );

		splitvect.push_back( tok );

		while( tok != 0 )
		{
			size_t ipos = (tok-buffer);

			tok = strtok( 0, pdelim );
			if( tok )
			{
				splitvect.push_back( tok );
			}
		}
		delete[] buffer;
	}
}
