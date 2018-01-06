struct Rapl;

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct Rapl* CRapl;
	CRapl create_rapl(int th);
	void rapl_before(CRapl);
	void rapl_after(int fn, CRapl);
	void show_power_info(CRapl);
	void show(CRapl);

#ifdef __cplusplus
}
#endif