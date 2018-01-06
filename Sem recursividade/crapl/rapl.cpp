  
/*
 *  Análise e Teste de Software
 *  João Saraiva 
 *  2016-2017
 */


#include "rapl.h"
#include <sched.h>
#include <ctime>

//time_t start,end;

int funct;

 Rapl::Rapl(int fn) {

  funct = fn;
  core = sched_getcpu();
  //thread = th;
  int fd;
  long long result;

  cpu_model=detect_cpu();
  if (cpu_model<0) {
   printf("Unsupported CPU type\n");
   //return -1;
 }

  // printf("Checking core #%d\n",core);


 fd=open_msr(core);

  /* Calculate the units used */
 result=read_msr(fd,MSR_RAPL_POWER_UNIT);

 power_units=pow(0.5,(double)(result&0xf));
 energy_units=pow(0.5,(double)((result>>8)&0x1f));
 time_units=pow(0.5,(double)((result>>16)&0xf));


 close(fd);

 reset();

 incrementCall(funct);

}


void Rapl::reset() {

  time_total += (double) end - begin; 
  pack_total += package_after;
  pp0_total += pp0_after;
  pp1_total += pp1_after;
  dram_total += dram_after;

  package_before = package_after = 0.0;
  pp0_before = pp0_after = 0.0;
  pp1_before = pp1_after = 0.0;
  dram_before = dram_after = 0.0;
  begin = end = 0.0;
}

int Rapl::open_msr(int core) {

  char msr_filename[BUFSIZ];
  int fd;

  //printf("Core: %d \n", core);
  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      exit(127);
    }
  }

  return fd;
}

long long Rapl::read_msr(int fd, int which) {

  uint64_t data;

  if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
    perror("rdmsr:pread");
    exit(127);
  }

  return (long long)data;
}



int Rapl::detect_cpu(void) {

	FILE *fff;

	int family,model=-1;
	char buffer[BUFSIZ],*result;
	char vendor[BUFSIZ];

	fff=fopen("/proc/cpuinfo","r");
	if (fff==NULL) return -1;

	while(1) {
		result=fgets(buffer,BUFSIZ,fff);
		if (result==NULL) break;

		if (!strncmp(result,"vendor_id",8)) {
			sscanf(result,"%*s%*s%s",vendor);

			if (strncmp(vendor,"GenuineIntel",12)) {
				printf("%s not an Intel chip\n",vendor);
				return -1;
			}
		}

		if (!strncmp(result,"cpu family",10)) {
			sscanf(result,"%*s%*s%*s%d",&family);
			if (family!=6) {
				printf("Wrong CPU family %d\n",family);
				return -1;
			}
		}

		if (!strncmp(result,"model",5)) {
			sscanf(result,"%*s%*s%d",&model);
		}

	}

	fclose(fff);

	/*switch(model) {
		case CPU_SANDYBRIDGE:
   printf("Found Sandybridge CPU\n");
   break;
   case CPU_SANDYBRIDGE_EP:
   printf("Found Sandybridge-EP CPU\n");
   break;
   case CPU_IVYBRIDGE:
   printf("Found Ivybridge CPU\n");
   break;
   case CPU_IVYBRIDGE_EP:
   printf("Found Ivybridge-EP CPU\n");
   break;
   case CPU_HASWELL:
   printf("Found Haswell CPU\n");
   break;
   case CPU_BROADWELL:
   printf("Found Broadwell CPU\n");
   break;
   default:	printf("Unsupported model %d\n",model);
   model=-1;
   break;
 }*/

   return model;
 }






 int Rapl::rapl_init(){ 

   printf("\nNºCore \t Thread \t PACKAGE \t CORE \t GPU \t DRAM \t TIME \n");

   return 0;
 }


 void Rapl::show_power_info(){ 
  //core = sched_getcpu();

  int fd;
  long long result;
  double thermal_spec_power,minimum_power,maximum_power,time_window;



 /* Show package power info */

  fd=open_msr(core);
  result=read_msr(fd,MSR_PKG_POWER_INFO);

  thermal_spec_power=power_units*(double)(result&0x7fff);
  printf("Package thermal spec: %.3fW\n",thermal_spec_power);

  minimum_power=power_units*(double)((result>>16)&0x7fff);
  printf("Package minimum power: %.3fW\n",minimum_power);

  maximum_power=power_units*(double)((result>>32)&0x7fff);
  printf("Package maximum power: %.3fW\n",maximum_power);

  time_window=time_units*(double)((result>>48)&0x7fff);
  printf("Package maximum time window: %.6fs\n",time_window);


  close(fd);
}



void Rapl::show_power_limit(){ 
  //int core = sched_getcpu();

  int fd;
  long long result;


 /* Show package power limit */

  fd=open_msr(core);
  result=read_msr(fd,MSR_PKG_RAPL_POWER_LIMIT);

  printf("Package power limits are %s\n", (result >> 63) ? "locked" : "unlocked");
  double pkg_power_limit_1 = power_units*(double)((result>>0)&0x7FFF);
  double pkg_time_window_1 = time_units*(double)((result>>17)&0x007F);
  printf("Package power limit #1: %.3fW for %.6fs (%s, %s)\n", pkg_power_limit_1, pkg_time_window_1,
   (result & (1LL<<15)) ? "enabled" : "disabled",
   (result & (1LL<<16)) ? "clamped" : "not_clamped");
  double pkg_power_limit_2 = power_units*(double)((result>>32)&0x7FFF);
  double pkg_time_window_2 = time_units*(double)((result>>49)&0x007F);
  printf("Package power limit #2: %.3fW for %.6fs (%s, %s)\n", pkg_power_limit_2, pkg_time_window_2,
    (result & (1LL<<47)) ? "enabled" : "disabled",
    (result & (1LL<<48)) ? "clamped" : "not_clamped");

  printf("\n");


  close(fd);

}

void Rapl::show(){

 //printf("Arroz %lu", sizeof(results));

  //printf("\nCORE: %d  -- THREAD: %d \n", core, thread);
/*
  printf("PACKAGE: %.6f \n", pack_total);

  printf("CORE: %.6f \n", pp0_total);

  printf("GPU: %.6f \n", pp1_total);

  printf("DRAM: %.6f \n", dram_total);

  printf("TIME: %.5f \n", (double)(time_total) / CLOCKS_PER_SEC);
*/
}


void Rapl::rapl_before(){

  if(checkRecursive(funct))
      return; 

  core = sched_getcpu();
	//time (&start);
        begin = clock();
	//gettimeofday(&tvb, NULL);
	

  int fd;
  long long result;
	
  fd=open_msr(core);
  result=read_msr(fd,MSR_PKG_ENERGY_STATUS);
  package_before=(double)result*energy_units;
  //printf("Package energy: %.6fJ\n",package_before[thread]);

  /* only available on *Bridge-EP */
  if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP))
  {
    result=read_msr(fd,MSR_PKG_PERF_STATUS);
    //double acc_pkg_throttled_time=(double)result*time_units;
    // fprintf(fp,"Accumulated Package Throttled Time : %.6fs\n",acc_pkg_throttled_time);
  }

  result=read_msr(fd,MSR_PP0_ENERGY_STATUS);
  pp0_before=(double)result*energy_units;
  //printf("PowerPlane0 (core) for core %d energy before: %.6fJ\n",core,pp0_before[thread]);

  result=read_msr(fd,MSR_PP0_POLICY);
  //int pp0_policy=(int)result&0x001f;
 // printf("PowerPlane0 (core) for core %d policy: %d\n",core,pp0_policy);

  /* only available on *Bridge-EP */
  if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP))
  {
    result=read_msr(fd,MSR_PP0_PERF_STATUS);
    //double acc_pp0_throttled_time=(double)result*time_units;
    //printf("PowerPlane0 (core) Accumulated Throttled Time : %.6fs\n",acc_pp0_throttled_time);
  }

  /* not available on *Bridge-EP */
  if ((cpu_model==CPU_SANDYBRIDGE) || (cpu_model==CPU_IVYBRIDGE) ||
   (cpu_model==CPU_HASWELL) || (cpu_model==CPU_BROADWELL)) {
   result=read_msr(fd,MSR_PP1_ENERGY_STATUS);
 pp1_before=(double)result*energy_units;
 //printf("PowerPlane1 (on-core GPU if avail) before: %.6fJ\n",pp1_before[thread]);
 result=read_msr(fd,MSR_PP1_POLICY);
 //int pp1_policy=(int)result&0x001f;
     //printf("PowerPlane1 (on-core GPU if avail) %d policy: %d\n",core,pp1_policy);
}

	/* Despite documentation saying otherwise, it looks like */
	/* You can get DRAM readings on regular Haswell          */
if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP) ||
 (cpu_model==CPU_HASWELL) || (cpu_model==CPU_BROADWELL)) {
 result=read_msr(fd,MSR_DRAM_ENERGY_STATUS);
dram_before=(double)result*energy_units;
//printf("%.6f\n", dram_before);
//printf("DRAM energy before: %.6fJ\n",dram_before[thread]);
}
close(fd);

//printf("Before: %.6f\n", rb);
}


void Rapl::rapl_after(int fn){

  if(checkRecursive(fn)){
      decrementRecur(fn);
      return; 
  }

  core = sched_getcpu();
  //int core = sched_getcpu();

  int fd;
  long long result;

  fd=open_msr(core);

  result=read_msr(fd,MSR_PKG_ENERGY_STATUS);
  package_after=(double)result*energy_units;
  //printf("\nPACKAGE: %.6f , %.6f \n",package_after, package_before);
  // PACKAGE

  result=read_msr(fd,MSR_PP0_ENERGY_STATUS);
  pp0_after=(double)result*energy_units; 
  //printf("CORE: %.6f , %.6f \n",pp0_after, pp0_before);// CORE
  
  
  /* not available on SandyBridge-EP */
  if ((cpu_model==CPU_SANDYBRIDGE) || (cpu_model==CPU_IVYBRIDGE) ||
   (cpu_model==CPU_HASWELL) || (cpu_model==CPU_BROADWELL)) {
   result=read_msr(fd,MSR_PP1_ENERGY_STATUS);
   pp1_after=(double)result*energy_units;    // GPU
 //printf("GPU: %.6f , %.6f \n",pp1_after, pp1_before);
}
else 
  printf("  , ");   

if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP) ||
 (cpu_model==CPU_HASWELL) || (cpu_model==CPU_BROADWELL)) {
 result=read_msr(fd,MSR_DRAM_ENERGY_STATUS);
 dram_after=(double)result*energy_units;
   //printf("DRAM: %.6f , %.6f \n\n",dram_after, dram_before);  // DRAM
}
else
  printf("  , ");  
close(fd);
	//time (&end);
	//double cl = difftime (end,start);
	end = clock();
	//gettimeofday(&tva, NULL);
	float cl = double(end - begin) / CLOCKS_PER_SEC;
	//double cl = ((tva.tv_sec-tvb.tv_sec) + (tva.tv_usec-tvb.tv_usec)/1000000.0);
	//printf("%.2lf\n", cl);

//printf("\nBefore: \%f - After: \%f \n", (double)begin, (double)end);

package_after -= package_before;
pp0_after -= pp0_before;
pp1_after -= pp1_before;
dram_after -= dram_before;


//printf("\t %.6f   ",package_after);
//printf("%.6f   ",pp0_after);  
//printf("%.6f   ",pp1_after); 
//printf("%.6f   ",dram_after); 
//printf("%.5f \n", cl);

//printf("%d    ", core);
//printf("\t %d   ", thread);
//nr_funcs--;
//results[nr_funcs][0] = package_after  - package_before;
//results[nr_funcs][1] = pp0_after - pp0_before;
//results[nr_funcs][2] = pp1_after - pp1_before;
//results[nr_funcs][3] = dram_after - dram_before;
//results[nr_funcs][4] = (double)(end - begin) / CLOCKS_PER_SEC;
if(package_before != 0)
	insertC(fn, package_after, pp0_after, pp1_after, dram_after, cl);
//insert(nr_funcs, 1, pp0_after - pp0_before);
//insert(nr_funcs, 2, pp1_after - pp1_before);
//insert(nr_funcs, 3, dram_after - dram_before);
//insert(nr_funcs, 4, (double)(end - begin) / CLOCKS_PER_SEC);

//printf("After: %.15f\n", ra);

reset();

}
