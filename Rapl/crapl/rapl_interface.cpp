#include "rapl.h"
#include "rapl_interface.h"


extern "C"
{
	CRapl create_rapl(int th) { return new Rapl(th); }
	void  rapl_before(CRapl p) { return p->rapl_before(); }
	void  rapl_after(int fn, CRapl p) { return p->rapl_after(fn); }
	void  show_power_info(CRapl p) { return p->show_power_info(); }
	void  show(CRapl p) { return p->show(); }
}