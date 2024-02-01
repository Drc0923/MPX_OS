#include <sys_req.h>
#include <yield.h>

void yield(void){
	sys_req(IDLE);
}
