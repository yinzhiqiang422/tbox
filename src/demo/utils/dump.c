/* ///////////////////////////////////////////////////////////////////////
 * includes
 */ 
#include "../demo.h"

/* ///////////////////////////////////////////////////////////////////////
 * main
 */ 
tb_int_t tb_demo_utils_dump_main(tb_int_t argc, tb_char_t** argv)
{
	// dump
	tb_dump_data_from_url(argv[1]);
	return 0;
}
